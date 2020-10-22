/*---------------------------------------------------------------------
  Implement Image Saving
  ---------------------------------------------------------------------*/

void io_image() {
  FILE *fp;
  char *f = string_extract(stack_pop());
  if ((fp = fopen(f, "wb")) == NULL) {
    printf("\nERROR (nga/io_image): Unable to save the image: %s!\n", f);
    exit(2);
  }
  fwrite(&memory, sizeof(CELL), memory[3] + 1, fp);
  fclose(fp);
}

void io_image_query() {
  stack_push(0);
  stack_push(1000);
}
