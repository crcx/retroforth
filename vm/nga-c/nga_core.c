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

#ifndef RETRO_NGA_CORE_H
#error "nga_core.c must be included after nga_core.h"
#endif

#ifndef BRANCH_PREDICTION
V guard(NgaState *vm, int n, int m, int diff) {
  if (ACTIVE.sp < n) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 0) {
      handle_error(vm, 1);
    }
#else
    printf("E: Data Stack Underflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (((ACTIVE.sp + m) - n) > (STACK_DEPTH - 1)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[2] != 0) {
      handle_error(vm, 2);
    }
#else
    printf("E: Data Stack Overflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (diff) {
    if (ACTIVE.rp + diff < 0) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[3] != 0) {
      handle_error(vm, 3);
    }
#else
      return;
#endif
    }
    if (ACTIVE.rp + diff > (ADDRESSES - 1)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 4) {
      handle_error(vm, 4);
    }
#else
      return;
#endif
    }
  }
}
#else
V guard(NgaState *vm, int n, int m, int diff) {
  if (unlikely(ACTIVE.sp < n)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 0) {
      handle_error(vm, 1);
    }
#else
    printf("E: Data Stack Underflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (unlikely(((ACTIVE.sp + m) - n) > (STACK_DEPTH - 1))) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[2] != 0) {
      handle_error(vm, 2);
    }
#else
    printf("E: Data Stack Overflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (unlikely(ACTIVE.rp + diff < 0)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[3] != 0) {
      handle_error(vm, 3);
    }
#else
      return;
#endif
  }
  if (unlikely(ACTIVE.rp + diff > (ADDRESSES - 1))) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 4) {
      handle_error(vm, 4);
    }
#else
    return;
#endif
  }
}
#endif

/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

V invalid_opcode(NgaState *vm, CELL opcode) {
  CELL a, i;
  printf("\nERROR (nga/execute): Invalid instruction!\n");
  printf("At %lld, opcode %lld\n", (long long)ACTIVE.ip, (long long)opcode);
  printf("Instructions: ");
  a = opcode;
  for (i = 0; i < 4; i++) {
    printf("%lldd ", (long long)a & 0xFF);
    a = a >> 8;
  }
  printf("\n");
  exit(1);
}

V execute(NgaState *vm, CELL cell) {
  CELL opcode;
  if (ACTIVE.rp == 0)
    ACTIVE.rp = 1;
  ACTIVE.ip = cell;
  while (ACTIVE.ip < IMAGE_SIZE) {
    if (vm->perform_abort == 0) {
      opcode = vm->memory[ACTIVE.ip];
#ifndef BRANCH_PREDICTION
      validate_opcode_bundle(vm, opcode);
#endif
      process_opcode_bundle(vm, opcode);
#ifndef ENABLE_ERROR
      if (ACTIVE.sp < 0 || ACTIVE.sp > STACK_DEPTH) {
        printf("\nERROR (nga/execute): Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. sp = %lld, core = %lld\n", (long long)ACTIVE.ip, (long long)opcode, (long long)ACTIVE.sp, (long long)vm->active);
        exit(1);
      }
      if (ACTIVE.rp < 0 || ACTIVE.rp > ADDRESSES) {
        printf("\nERROR (nga/execute): Address Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. rp = %lld\n", (long long)ACTIVE.ip, (long long)opcode, (long long)ACTIVE.rp);
        exit(1);
      }
#endif
      ACTIVE.ip++;
#ifdef ENABLE_MULTICORE
      switch_core(vm);
#endif
      if (ACTIVE.rp == 0)
        ACTIVE.ip = IMAGE_SIZE;
    } else {
      carry_out_abort(vm);
    }
  }
}

/*---------------------------------------------------------------------
  Interfacing With The Image
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop(NgaState *vm) {
  ACTIVE.sp--;
  return ACTIVE.data[ACTIVE.sp + 1];
}

V stack_push(NgaState *vm, CELL value) {
  ACTIVE.sp++;
  ACTIVE.data[ACTIVE.sp] = value;
}


/*---------------------------------------------------------------------
  This interface tracks a few words and variables in the image. These
  are:

  Dictionary - the latest dictionary header
  interpret  - the heart of the interpreter/compiler

  I have to call this periodically, as the Dictionary will change as
  new words are defined, and the user might write a new error handler
  or interpreter.
  ---------------------------------------------------------------------*/

V update_rx(NgaState *vm) {
  vm->Dictionary = vm->memory[RETRO_IMAGE_DICTIONARY];
  vm->interpret = vm->memory[RETRO_IMAGE_INTERPRET];
  if (vm->memory[RETRO_IMAGE_DICTREHASH] != 0) {
    execute(vm, vm->memory[RETRO_IMAGE_DICTREHASH]);
  }
}

/*=====================================================================*/

V register_device(NgaState *vm, Handler handler, Handler query) {
  vm->IO_deviceHandlers[vm->devices] = handler;
  vm->IO_queryHandlers[vm->devices] = query;
  vm->devices++;
}

V load_embedded_image(NgaState *vm) {
  int i;
  for (i = 0; i < ngaImageCells; i++)
    vm->memory[i] = ngaImage[i];
}

CELL load_image(NgaState *vm, char *imageFile) {
  FILE *fp;
  CELL imageSize = 0;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    if (fileLen > IMAGE_SIZE) {
      fclose(fp);
      printf("\nERROR (nga/ngaLoadImage): Image is larger than alloted space!\n");
      exit(1);
    }
    rewind(fp);

    /* Erase old image in memory: 0 = nop instruction */
    for (int i = 0; i < IMAGE_SIZE; i++) { vm->memory[i] = 0; }

    /* Read the file into memory */
    imageSize = fread(vm->memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  return imageSize;
}

V prepare_vm(NgaState *vm) {
  vm->active = 0;
  ACTIVE.ip = ACTIVE.sp = ACTIVE.rp = ACTIVE.u = 0;
  ACTIVE.active = -1;
  for (ACTIVE.ip = 0; ACTIVE.ip < IMAGE_SIZE; ACTIVE.ip++)
    vm->memory[ACTIVE.ip] = 0; /* NO - nop instruction */
  for (ACTIVE.ip = 0; ACTIVE.ip < STACK_DEPTH; ACTIVE.ip++)
    ACTIVE.data[ACTIVE.ip] = 0;
  for (ACTIVE.ip = 0; ACTIVE.ip < ADDRESSES; ACTIVE.ip++)
    ACTIVE.address[ACTIVE.ip] = 0;
}

V i_no(NgaState *vm) {
#ifndef BRANCH_PREDICTION
  guard(vm, 0, 0, 0);
#endif
}

V i_li(NgaState *vm) {
  guard(vm, 0, 1, 0);
  ACTIVE.sp++;
  ACTIVE.ip++;
  TOS = vm->memory[ACTIVE.ip];
}

V i_du(NgaState *vm) {
  guard(vm, 1, 2, 0);
  ACTIVE.sp++;
  ACTIVE.data[ACTIVE.sp] = NOS;
}

V i_dr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.data[ACTIVE.sp] = 0;
  ACTIVE.sp--;
}

V i_sw(NgaState *vm) {
  guard(vm, 2, 2, 0);
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

V i_pu(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = TOS;
  i_dr(vm);
}

V i_po(NgaState *vm) {
  guard(vm, 0, 1, -1);
  ACTIVE.sp++;
  TOS = TORS;
  ACTIVE.rp--;
}

V i_ju(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.ip = TOS - 1;
  i_dr(vm);
}

V i_ca(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = ACTIVE.ip;
  ACTIVE.ip = TOS - 1;
  i_dr(vm);
}

V i_cc(NgaState *vm) {
  guard(vm, 2, 0, 1);
  CELL a, b;
  a = TOS; i_dr(vm);  /* Target */
  b = TOS; i_dr(vm);  /* Flag   */
  if (b != 0) {
    ACTIVE.rp++;
    TORS = ACTIVE.ip;
    ACTIVE.ip = a - 1;
  }
}

V i_re(NgaState *vm) {
  guard(vm, 0, 0, -1);
  ACTIVE.ip = TORS;
  ACTIVE.rp--;
}

V i_eq(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS == (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS == TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_ne(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS != (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS != TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_lt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS < (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS < TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_gt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS > (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS > TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_fe(NgaState *vm) {
  guard(vm, 1, 1, 0);
  switch (TOS) {
    case -1: TOS = ACTIVE.sp - 1; break;
    case -2: TOS = ACTIVE.rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: TOS = vm->memory[TOS]; break;
  }
}

V i_st(NgaState *vm) {
  guard(vm, 2, 0, 0);
  vm->memory[TOS] = NOS;
  i_dr(vm);
  i_dr(vm);
}

V i_ad(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS + (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS += TOS;
  }
  i_dr(vm);
}

V i_su(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS - (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS -= TOS;
  }
  i_dr(vm);
}

V i_mu(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS * (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS *= TOS;
  }
  i_dr(vm);
}

V i_di(NgaState *vm) {
  guard(vm, 2, 2, 0);
  CELL a, b;
  a = TOS;
  b = NOS;
  if (b == 0) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[6] != 0) {
    }
#endif
  }
  if (ACTIVE.u != 0) {
    TOS = (unsigned)b / (unsigned)a;
    NOS = (unsigned)b % (unsigned)a;
    ACTIVE.u = 0;
  } else {
    TOS = b / a;
    NOS = b % a;
  }
}

V i_an(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS & NOS;
  i_dr(vm);
}

V i_or(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS | NOS;
  i_dr(vm);
}

V i_xo(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS ^ NOS;
  i_dr(vm);
}

V i_sh(NgaState *vm) {
  guard(vm, 2, 1, 0);
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (0 - TOS);
  else {
    if (ACTIVE.u != 0) {
      NOS = (unsigned)x >> (unsigned)y;
      ACTIVE.u = 0;
    } else {
      if (x < 0 && y > 0)
        NOS = x >> y | ~(~0U >> y);
      else
        NOS = x >> y;
    }
  }
  i_dr(vm);
}

V i_zr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  if (TOS == 0) {
    i_dr(vm);
    ACTIVE.ip = TORS;
    ACTIVE.rp--;
  }
}

V i_ha(NgaState *vm) {
  guard(vm, 0, 0, 0);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
  exit(0);
}

V i_ie(NgaState *vm) {
  guard(vm, 1, 1, 0);
  stack_push(vm, vm->devices);
}

int valid_device(NgaState *vm, CELL device) {
  return device >= 0 && device < vm->devices && device < MAX_DEVICES;
}

V invalid_device(NgaState *vm, CELL device) {
#ifdef ENABLE_ERROR
  if (vm->ErrorHandlers[5] != 0) {
    handle_error(vm, 5);
    return;
  }
#endif
  printf("\nERROR (nga/device): Invalid device id %lld\n", (long long)device);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V i_iq(NgaState *vm) {
  guard(vm, 1, 1, 0);
  CELL device = stack_pop(vm);
  if (valid_device(vm, device)) {
    vm->IO_queryHandlers[device](vm);
  } else {
    invalid_device(vm, device);
  }
}

V i_ii(NgaState *vm) {
  guard(vm, 1, 0, 0);
  CELL device = stack_pop(vm);
  if (valid_device(vm, device)) {
    vm->IO_deviceHandlers[device](vm);
  } else {
    invalid_device(vm, device);
  }
}

Handler instructions[] = {
  i_no, i_li, i_du, i_dr, i_sw, i_pu, i_po,
  i_ju, i_ca, i_cc, i_re, i_eq, i_ne, i_lt,
  i_gt, i_fe, i_st, i_ad, i_su, i_mu, i_di,
  i_an, i_or, i_xo, i_sh, i_zr, i_ha, i_ie,
  i_iq, i_ii
};

#ifndef BRANCH_PREDICTION
V validate_opcode_bundle(NgaState *vm, CELL opcode) {
  CELL remainingOpcode = opcode;
  for (int i = 0; i < 4; i++) {
    CELL current = remainingOpcode & 0xFF;
    if (current < 0 || current > 29) {
      invalid_opcode(vm, opcode);
    }
    remainingOpcode >>= 8;
  }
}
#endif

#define INST(n) ((opcode >> n) & 0xFF) != 0
V process_opcode_bundle(NgaState *vm, CELL opcode) {
#ifndef BRANCH_PREDICTION
  if (INST(0))  instructions[opcode & 0xFF](vm);
  if (INST(8))  instructions[(opcode >> 8) & 0xFF](vm);
  if (INST(16)) instructions[(opcode >> 16) & 0xFF](vm);
  if (INST(24)) instructions[(opcode >> 24) & 0xFF](vm);
#else
  for (size_t i = 0; i < 4; ++i) {
    uint8_t current = ((uint8_t*)&opcode)[i];
    if (unlikely(current > 29)) {
      invalid_opcode(vm, opcode);
    }
    instructions[current](vm);
  }
#endif
}

#define RETRO_NGA_CORE_IMPLEMENTED 1
