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

#include "utf32.c"

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

CELL string_inject(NgaState *vm, char *str, CELL buffer) {
  if (!str) {
    vm->memory[buffer] = 0;
    return 0;
  }

  CELL pos = 0;
  int i = 0;

  while (str[i] != '\0') {
    unsigned char *utf8_str = (unsigned char *)&str[i];
    int utf32_char = 0;
    int bytes_consumed = 0;

    // Convert UTF-8 to UTF-32
    if ((utf8_str[0] & 0x80) == 0x00) {
      // 1-byte UTF-8 sequence
      utf32_char = utf8_str[0];
      bytes_consumed = 1;
    } else if ((utf8_str[0] & 0xE0) == 0xC0) {
      // 2-byte UTF-8 sequence
      utf32_char = ((uint32_t)(utf8_str[0] & 0x1F) << 6) |
                              (utf8_str[1] & 0x3F);
      bytes_consumed = 2;
    } else if ((utf8_str[0] & 0xF0) == 0xE0) {
      // 3-byte UTF-8 sequence
      utf32_char = ((uint32_t)(utf8_str[0] & 0x0F) << 12) |
                   ((uint32_t)(utf8_str[1] & 0x3F) << 6) |
                              (utf8_str[2] & 0x3F);
      bytes_consumed = 3;
    } else if ((utf8_str[0] & 0xF8) == 0xF0) {
      // 4-byte UTF-8 sequence
      utf32_char = ((uint32_t)(utf8_str[0] & 0x07) << 18) |
                   ((uint32_t)(utf8_str[1] & 0x3F) << 12) |
                   ((uint32_t)(utf8_str[2] & 0x3F) << 6) |
                              (utf8_str[3] & 0x3F);
      bytes_consumed = 4;
    } else {
      // Invalid UTF-8, skip byte
      utf32_char = utf8_str[0];
      bytes_consumed = 1;
    }

    vm->memory[buffer + pos] = (CELL)utf32_char;
    vm->memory[buffer + pos + 1] = 0;
    pos++;
    i += bytes_consumed;
  }

  return buffer;
}

char *string_extract(NgaState *vm, CELL at) {
  CELL starting = at, i = 0;
  while (vm->memory[starting] && i < 8192)
    vm->string_data[i++] = (char)vm->memory[starting++];
  vm->string_data[i] = 0;
  return (char *)vm->string_data;
}
