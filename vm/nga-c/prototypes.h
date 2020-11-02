/*---------------------------------------------------------------------
  Function Prototypes
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
CELL string_inject(char *str, CELL buffer);
char *string_extract(CELL at);
CELL d_xt_for(char *Name, CELL Dictionary);
void update_rx();
void include_file(char *fname, int run_tests);
void io_output_handler();
void io_output_query();
void io_floatingpoint_handler();
void io_floatingpoint_query();
void io_keyboard_handler();
void io_keyboard_query();
void io_filesystem_query();
void io_filesystem_handler();
void io_unix_query();
void io_unix_handler();
void io_clock_handler();
void io_clock_query();
void io_scripting_handler();
void io_scripting_query();
void io_image();
void io_image_query();
void io_random();
void io_random_query();

void io_socket();

CELL load_image();
void prepare_vm();
void process_opcode(CELL opcode);
void process_opcode_bundle(CELL opcode);
int validate_opcode_bundle(CELL opcode);

size_t bsd_strlcat(char *dst, const char *src, size_t dsize);
size_t bsd_strlcpy(char *dst, const char *src, size_t dsize);
