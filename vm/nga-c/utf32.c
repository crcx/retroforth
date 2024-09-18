int utf32_length(int *s, int max) {
  int l = 0;
  while (*s++ != 0 && l < max) l++;
  return l;
}
