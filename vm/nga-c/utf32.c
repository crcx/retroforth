size_t utf32_strlen(const int32_t *utf32_str) {
  size_t length = 0;
  while (utf32_str[length] != 0) {
    length++;
  }
  return length;
}

void utf32_strcpy(int32_t *dest, const int32_t *src) {
  while ((*dest++ = *src++) != 0);
}

int utf32_strcmp(const int32_t *str1, const int32_t *str2) {
  while (*str1 && (*str1 == *str2)) {
    str1++;
    str2++;
  }
  return *str1 - *str2;
}

void c_to_utf32(const char *source, int32_t *dest, int max) {
  int i = 0;
  while (source[i] != '\0' && i < max - 1) {
    dest[i] = (int32_t)source[i];
    i++;
  }
  dest[i] = 0;
}

void utf32_to_c(const int32_t *source, char *dest, int max) {
  int i = 0;
  while (source[i] != 0 && i < max - 1) {
    if (source[i] <= 0x7F) {
      dest[i] = (char)source[i];
    } else {
      dest[i] = '?';
    }
    i++;
  }
  dest[i] = '\0';
}
