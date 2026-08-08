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
#error "string_handling.c must be included after nga_core.c"
#endif

V display_utf8(const unsigned char* utf8_bytes, int num_bytes) {
    if (write(STDOUT_FILENO, utf8_bytes, num_bytes) == -1) {
        perror("Error writing to /dev/stdout");
    }
}

V utf32_to_utf8(uint32_t utf32_char, unsigned char* utf8_bytes, int* num_bytes) {
    if (utf32_char < 0x80) {
        utf8_bytes[0] = (unsigned char)utf32_char;
        *num_bytes = 1;
    } else if (utf32_char < 0x800) {
        utf8_bytes[0] = (unsigned char)(0xC0 | (utf32_char >> 6));
        utf8_bytes[1] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 2;
    } else if (utf32_char < 0x10000) {
        utf8_bytes[0] = (unsigned char)(0xE0 | (utf32_char >> 12));
        utf8_bytes[1] = (unsigned char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_bytes[2] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 3;
    } else if (utf32_char < 0x110000) {
        utf8_bytes[0] = (unsigned char)(0xF0 | (utf32_char >> 18));
        utf8_bytes[1] = (unsigned char)(0x80 | ((utf32_char >> 12) & 0x3F));
        utf8_bytes[2] = (unsigned char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_bytes[3] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 4;
    } else {
        *num_bytes = 0;
    }
}

int read_character(int from) {
  unsigned char utf8_bytes[4] = { 0 };
  int utf32_char, i, num_bytes;

  if (read(from, &utf8_bytes[0], 1) != 1) { return 0; }
  if ((utf8_bytes[0] & 0x80) == 0x00) {
    num_bytes = 1;
  } else if ((utf8_bytes[0] & 0xE0) == 0xC0) {
    num_bytes = 2;
  } else if ((utf8_bytes[0] & 0xF0) == 0xE0) {
    num_bytes = 3;
  } else if ((utf8_bytes[0] & 0xF8) == 0xF0) {
    num_bytes = 4;
  } else {
    return 0;
  }

  for (i = 1; i < num_bytes; i++) {
    if (read(from, &utf8_bytes[i], 1) != 1) {
      return 0;
    }
  }

  if (num_bytes == 1) {
    utf32_char = utf8_bytes[0];
  } else if (num_bytes == 2) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x1F) << 6) |
                            (utf8_bytes[1] & 0x3F);
  } else if (num_bytes == 3) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x0F) << 12) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 6) |
                            (utf8_bytes[2] & 0x3F);
  } else if (num_bytes == 4) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x07) << 18) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 12) |
                 ((uint32_t)(utf8_bytes[2] & 0x3F) << 6) |
                            (utf8_bytes[3] & 0x3F);
  } else {
    return 0;
  }
  return utf32_char;
}

int fread_character(FILE *from) {
  unsigned char utf8_bytes[4] = { 0 };
  int utf32_char, i, num_bytes;

  if (fread(&utf8_bytes[0], 1, 1, from) != 1) { return 0; }
  if ((utf8_bytes[0] & 0x80) == 0x00) {
    num_bytes = 1;
  } else if ((utf8_bytes[0] & 0xE0) == 0xC0) {
    num_bytes = 2;
  } else if ((utf8_bytes[0] & 0xF0) == 0xE0) {
    num_bytes = 3;
  } else if ((utf8_bytes[0] & 0xF8) == 0xF0) {
    num_bytes = 4;
  } else {
    return 0;
  }

  for (i = 1; i < num_bytes; i++) {
    if (fread(&utf8_bytes[i], 1, 1, from) != 1) {
      return 0;
    }
  }

  if (num_bytes == 1) {
    utf32_char = utf8_bytes[0];
  } else if (num_bytes == 2) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x1F) << 6) |
                            (utf8_bytes[1] & 0x3F);
  } else if (num_bytes == 3) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x0F) << 12) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 6) |
                            (utf8_bytes[2] & 0x3F);
  } else if (num_bytes == 4) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x07) << 18) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 12) |
                 ((uint32_t)(utf8_bytes[2] & 0x3F) << 6) |
                            (utf8_bytes[3] & 0x3F);
  } else {
    return 0;
  }
  return utf32_char;
}

V string_memory_error(const char *operation) {
  fprintf(stderr, "ERROR (nga/%s): Invalid memory range\n", operation);
}

size_t decode_utf8_character(const unsigned char *input, size_t remaining,
                             uint32_t *character) {
  if ((input[0] & 0x80) == 0x00) {
    *character = input[0];
    return 1;
  }
  if ((input[0] & 0xE0) == 0xC0 && remaining >= 2) {
    *character = ((uint32_t)(input[0] & 0x1F) << 6) |
                 (input[1] & 0x3F);
    return 2;
  }
  if ((input[0] & 0xF0) == 0xE0 && remaining >= 3) {
    *character = ((uint32_t)(input[0] & 0x0F) << 12) |
                 ((uint32_t)(input[1] & 0x3F) << 6) |
                 (input[2] & 0x3F);
    return 3;
  }
  if ((input[0] & 0xF8) == 0xF0 && remaining >= 4) {
    *character = ((uint32_t)(input[0] & 0x07) << 18) |
                 ((uint32_t)(input[1] & 0x3F) << 12) |
                 ((uint32_t)(input[2] & 0x3F) << 6) |
                 (input[3] & 0x3F);
    return 4;
  }
  *character = input[0];
  return 1;
}

CELL string_inject(NgaState *vm, char *str, CELL buffer) {
  const unsigned char *input;
  size_t input_length, offset, cells;

  if (buffer < 0 || buffer >= IMAGE_SIZE) {
    string_memory_error("string_inject");
    return 0;
  }
  if (!str) {
    vm->memory[buffer] = 0;
    return 0;
  }

  input = (const unsigned char *)str;
  input_length = strlen(str);
  for (offset = cells = 0; offset < input_length; cells++) {
    uint32_t character;
    offset += decode_utf8_character(input + offset, input_length - offset,
                                    &character);
  }
  if (cells >= (size_t)(IMAGE_SIZE - buffer)) {
    string_memory_error("string_inject");
    return 0;
  }

  for (offset = cells = 0; offset < input_length; cells++) {
    uint32_t character;
    offset += decode_utf8_character(input + offset, input_length - offset,
                                    &character);
    vm->memory[buffer + cells] = (CELL)character;
  }
  vm->memory[buffer + cells] = 0;
  return buffer;
}

char *string_extract(NgaState *vm, CELL at) {
  CELL i = 0;
  if (at < 0 || at >= IMAGE_SIZE) {
    string_memory_error("string_extract");
    vm->string_data[0] = 0;
    return vm->string_data;
  }
  while (at < IMAGE_SIZE && vm->memory[at] &&
         i < (CELL)sizeof(vm->string_data) - 1) {
    vm->string_data[i++] = (char)vm->memory[at++];
  }
  vm->string_data[i] = 0;
  return vm->string_data;
}

#define RETRO_STRING_HANDLING_IMPLEMENTED 1
