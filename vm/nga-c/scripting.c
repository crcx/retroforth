/**************************************************************
                _              __            _   _
       _ __ ___| |_ _ __ ___  / _| ___  _ __| |_| |__
      | '__/ _ \ __| '__/ _ \| |_ / _ \| '__| __| '_ \
      | | |  __/ |_| | | (_) |  _| (_) | |  | |_| | | |
      |_|  \___|\__|_|  \___/|_|  \___/|_|   \__|_| |_|
                                                for nga

      (c) Charles Childers, Luke Parrish, Marc Simpsonn,
          Jay Skeer, Kenneth Keating

**************************************************************/

#ifndef RETRO_NGA_CORE_IMPLEMENTED
#error "scripting.c must be included after nga_core.c"
#endif

#ifndef RETRO_STRING_HANDLING_IMPLEMENTED
#error "scripting.c must be included after string_handling.c"
#endif

/*---------------------------------------------------------------------
  Scripting Support
  ---------------------------------------------------------------------*/

V scripting_arg(NgaState *vm) {
  CELL a, b;
  a = stack_pop(vm);
  b = stack_pop(vm);
  stack_push(vm, string_inject(vm, vm->sys_argv[a + 2], b));
}

V scripting_arg_count(NgaState *vm) {
  if ((vm->sys_argc - 2) <= 0)
    stack_push(vm, 0);
  else
    stack_push(vm, vm->sys_argc - 2);
}

V scripting_include(NgaState *vm) {
  include_file(vm, string_extract(vm, stack_pop(vm)), 0);
}

V scripting_include_plain(NgaState *vm) {
  include_plain_file(vm, string_extract(vm, stack_pop(vm)), 0);
}

V scripting_name(NgaState *vm) {
  if (vm->sys_argc == 1)
    stack_push(vm, string_inject(vm, "<none>", stack_pop(vm)));
  else
    stack_push(vm, string_inject(vm, vm->sys_argv[1], stack_pop(vm)));
}

/* addeded in scripting i/o device, revision 1 */
V scripting_source(NgaState *vm) {
  stack_push(vm, string_inject(vm, vm->scripting_sources[vm->current_source], stack_pop(vm)));
}

V scripting_line(NgaState *vm) {
  stack_push(vm, vm->currentLine);
}

V scripting_ignore_to_eol(NgaState *vm) {
  vm->ignoreToEOL = -1;
}

V scripting_ignore_to_eof(NgaState *vm) {
  vm->ignoreToEOF = -1;
}

V scripting_abort(NgaState *vm) {
  scripting_ignore_to_eol(vm);
  scripting_ignore_to_eof(vm);
  vm->perform_abort = -1;
}

V carry_out_abort(NgaState *vm) {
  ACTIVE.ip = IMAGE_SIZE + 1;
  ACTIVE.rp = 0;
  ACTIVE.sp = 0;
#ifdef ENABLE_FLOATS
  vm->fsp = 0;
  vm->afsp = 0;
#endif

  if (vm->current_source > 0) {
    scripting_abort(vm);
    return;
  }

  vm->perform_abort = 0;
  vm->current_source = 0;
}

V scripting_line_text(NgaState *vm) {
  CELL target = stack_pop(vm);
  string_inject(vm, vm->line, target);
}

Handler ScriptingActions[] = {
  scripting_arg_count,     scripting_arg,
  scripting_include,       scripting_name,
  scripting_source,        scripting_line,
  scripting_ignore_to_eol, scripting_ignore_to_eof,
  scripting_abort,         scripting_line_text,
  scripting_include_plain
};

V query_scripting(NgaState *vm) {
  stack_push(vm, 3);
  stack_push(vm, DEVICE_SCRIPTING);
}

V invalid_scripting_action(NgaState *vm, CELL action) {
  printf("\nERROR (nga/scripting): Invalid scripting action %lld\n", (long long)action);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V io_scripting(NgaState *vm) {
  CELL action = stack_pop(vm);
  CELL actions = sizeof(ScriptingActions) / sizeof(ScriptingActions[0]);
  if (action >= 0 && action < actions) {
    ScriptingActions[action](vm);
  } else {
    invalid_scripting_action(vm, action);
  }
}


/*=====================================================================*/

/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

V evaluate(NgaState *vm, char *s) {
  if (strlen(s) == 0)  return;
  if (strlen(s) > (TIB_END  - TIB)) {
    s[TIB_END - TIB] = 0;
  }
  string_inject(vm, s, TIB);
  stack_push(vm, TIB);
  execute(vm, vm->interpret);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int c) {
  return (c != 9) && (c != 10) && (c != 13) && (c != 32) && (c != EOF) && (c != 0);
}

V read_token(FILE *file, char *token_buffer) {
  int ch = fread_character(file);
  int count = 0;
  unsigned char utf8_bytes[4];
  int num_bytes;

  while (not_eol(ch)) {
    if ((ch == 8 || ch == 127) && count > 0) {
      // Handle backspace - need to find start of previous UTF-8 character
      count--;
      while (count > 0 && (token_buffer[count] & 0xC0) == 0x80) {
        count--; // Skip continuation bytes
      }
    } else {
      // Convert UTF-32 character to UTF-8 bytes and store
      utf32_to_utf8((uint32_t)ch, utf8_bytes, &num_bytes);
      for (int i = 0; i < num_bytes && count < 65535; i++) {
        token_buffer[count++] = utf8_bytes[i];
      }
    }
    ch = fread_character(file);
  }
  token_buffer[count] = '\0';
}


V skip_indent(FILE *fp) {
  int ch = getc(fp);
  while (ch == ' ') {
    ch = getc(fp);
  }
  ungetc(ch, fp);
}

/*---------------------------------------------------------------------
  RRE is primarily intended to be used in a batch or scripting model.
  The `include_file()` function will be used to read the code in the
  file, evaluating it as encountered.

  I enforce a literate model, with code in fenced blocks. E.g.,

    # This is a test

    Display "Hello, World!"

    ~~~
    'Hello,_World! puts nl
    ~~~

  RRE will ignore anything outside the `~~~` blocks. To identify if the
  current token is the start or end of a block, I provide a `fenced()`
  function.
  ---------------------------------------------------------------------*/

/* Check to see if a line is a fence boundary.
   This will check code blocks in all cases, and test blocks
   if tests_enabled is set to a non-zero value. */

int fence_boundary(NgaState *vm, char *buffer, int tests_enabled) {
  int flag = 1;
  if (strcmp(buffer, vm->code_start) == 0) { flag = -1; }
  if (strcmp(buffer, vm->code_end) == 0)   { flag = -1; }
  if (strcmp(buffer, vm->test_start) == 0) {
    if (vm->codeBlocks == 0) { vm->codeBlocks++; }
  }
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, vm->test_start) == 0) { flag = -1; }
  if (strcmp(buffer, vm->test_end) == 0)   { flag = -1; }
  return flag;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

int read_line(NgaState *vm, FILE *file, char *token_buffer) {
  int ch = getc(file);
  int count = 0;
  int tokens = 1;
  token_buffer[0] = '\0';
  while ((ch != 10) && (ch != 13) && (ch != EOF) && (ch != 0)) {
    if (count < (int)sizeof(vm->line) - 1) {
      token_buffer[count++] = ch;
    }
    ch = fread_character(file);
    if (ch != 10 && ch != 13 && ch != EOF && ch != 0 && isspace((unsigned char)ch)) {
      tokens++;
    }
  }
  token_buffer[count] = '\0';
  return tokens;
}

V include_file(NgaState *vm, char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  int priorBlocks = 0;
  char source[64 * 1024];          /* Token buffer [about 64K]         */
  char fence[33];                  /* Used with `fence_boundary()`     */

  CELL ReturnStack[ADDRESSES];
  CELL arp, aip;

  long offset = 0;
  CELL at = 0;
  int tokens = 0;
  FILE *fp;                        /* Open the file. If not found,     */
  if (vm->current_source >= MAX_SCRIPTING_SOURCES - 1) {
    printf("Maximum source include depth exceeded. Exiting.\n");
    exit(1);
  }

  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL) {
    printf("File `%s` not found. Exiting.\n", fname);
    exit(1);
  }

  priorBlocks = vm->codeBlocks;
  vm->codeBlocks = 0;

  arp = ACTIVE.rp;
  aip = ACTIVE.ip;
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ReturnStack[ACTIVE.rp] = ACTIVE.address[ACTIVE.rp];
  ACTIVE.rp = 0;

  vm->current_source++;
  strlcpy(vm->scripting_sources[vm->current_source], fname, 8192);

  vm->ignoreToEOF = 0;

  while (!feof(fp) && (vm->ignoreToEOF == 0)) { /* Loop through the file   */

    vm->ignoreToEOL = 0;

    offset = ftell(fp);
    tokens = read_line(vm, fp, vm->line);
    at++;
    fseek(fp, offset, SEEK_SET);
    skip_indent(fp);

    while (tokens > 0 && vm->ignoreToEOL == 0) {
      tokens--;
      read_token(fp, source);
      strlcpy(fence, source, 32); /* Copy the first three characters  */
      if (fence_boundary(vm, fence, run_tests) == -1) {
        if (inBlock == 0) {
          inBlock = 1;
          vm->codeBlocks++;
        } else {
          inBlock = 0;
        }
      } else {
        if (inBlock == 1) {
          vm->currentLine = at;
          evaluate(vm, source);
          vm->currentLine = at;
        }
      }
    }
    if (vm->ignoreToEOL == -1) {
      read_line(vm, fp, vm->line);
    }
  }

  vm->current_source--;
  vm->ignoreToEOF = 0;
  fclose(fp);
  if (vm->perform_abort == -1) {
    carry_out_abort(vm);
  }
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ACTIVE.address[ACTIVE.rp] = ReturnStack[ACTIVE.rp];
  ACTIVE.rp = arp;
  ACTIVE.ip = aip;

  if (vm->codeBlocks == 0) {
    printf("warning: no code or test blocks found!\n");
    printf("         filename: %s\n", fname);
    printf("         see http://unu.retroforth.org for a brief summary of\n");
    printf("         the unu code format used by retro\n");

  }
  vm->codeBlocks = priorBlocks;
}


V include_plain_file(NgaState *vm, char *fname, int run_tests) {
  char source[64 * 1024];          /* Token buffer [about 64K]         */

  CELL ReturnStack[ADDRESSES];
  CELL arp, aip;

  long offset = 0;
  CELL at = 0;
  int tokens = 0;
  FILE *fp;                        /* Open the file. If not found,     */
  if (vm->current_source >= MAX_SCRIPTING_SOURCES - 1) {
    printf("Maximum source include depth exceeded. Exiting.\n");
    exit(1);
  }

  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL) {
    printf("File `%s` not found. Exiting.\n", fname);
    exit(1);
  }

  arp = ACTIVE.rp;
  aip = ACTIVE.ip;
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ReturnStack[ACTIVE.rp] = ACTIVE.address[ACTIVE.rp];
  ACTIVE.rp = 0;

  vm->current_source++;
  strlcpy(vm->scripting_sources[vm->current_source], fname, 8192);

  vm->ignoreToEOF = 0;

  while (!feof(fp) && (vm->ignoreToEOF == 0)) { /* Loop through the file   */

    vm->ignoreToEOL = 0;

    offset = ftell(fp);
    tokens = read_line(vm, fp, vm->line);
    at++;
    fseek(fp, offset, SEEK_SET);
    skip_indent(fp);

    while (tokens > 0 && vm->ignoreToEOL == 0) {
      tokens--;
      read_token(fp, source);
      vm->currentLine = at;
      evaluate(vm, source);
      vm->currentLine = at;
    }
    if (vm->ignoreToEOL == -1) {
      read_line(vm, fp, vm->line);
    }
  }

  vm->current_source--;
  vm->ignoreToEOF = 0;
  fclose(fp);
  if (vm->perform_abort == -1) {
    carry_out_abort(vm);
  }
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ACTIVE.address[ACTIVE.rp] = ReturnStack[ACTIVE.rp];
  ACTIVE.rp = arp;
  ACTIVE.ip = aip;
}

V initialize_scripting(NgaState *vm) {
  vm->interactive = 0;

  strlcpy(vm->code_start, "~~~", 256);
  strlcpy(vm->code_end,   "~~~", 256);
  strlcpy(vm->test_start, "```", 256);
  strlcpy(vm->test_end,   "```", 256);

  /* Setup variables related to the scripting device */
  vm->currentLine = 0;           /* Current Line # for script */
  vm->current_source = 0;        /* Current file being run    */
  vm->perform_abort = 0;         /* Carry out abort procedure */
  strlcpy(vm->scripting_sources[0], "/dev/stdin", 8192);
  vm->ignoreToEOL = 0;
  vm->ignoreToEOF = 0;
  vm->codeBlocks = 0;
}
