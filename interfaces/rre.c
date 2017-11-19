/* RETRO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   a personal, pragmatic, minimalistic forth
   Copyright (c) 2016, 2017 Charles Childers

   This is `rre`, short for `run retro and exit`. It's the basic
   interface layer for Retro on FreeBSD, Linux and macOS.

   rre embeds the image file into the binary, so the compiled version
   of this is all you need to have a functional system.
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

/* Configure Nga (the VM) Limitations */
#define CELL         int32_t
#define IMAGE_SIZE   524288 * 16
#define ADDRESSES    2048
#define STACK_DEPTH  512

/* This assumes some knowledge of the ngaImage format for the
   Retro language. If things change there, these will need to
   be adjusted to match. */

#define TIB            1025
#define D_OFFSET_LINK     0
#define D_OFFSET_XT       1
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

/* More Nga bits. You shouldn't need to alter these */

enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_END
};
#define NUM_OPS VM_END + 1

CELL sp, rp, ip;
CELL data[STACK_DEPTH];
CELL address[ADDRESSES];
CELL memory[IMAGE_SIZE + 1];
#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]

/* Function Prototypes */

CELL stack_pop();
void stack_push(CELL value);
int string_inject(char *str, int buffer);
char *string_extract(int at);
int d_link(CELL dt);
int d_xt(CELL dt);
int d_class(CELL dt);
int d_name(CELL dt);
int d_lookup(CELL Dictionary, char *name);
CELL d_xt_for(char *Name, CELL Dictionary);
CELL d_class_for(char *Name, CELL Dictionary);
CELL ioGetFileHandle();
CELL ioOpenFile();
CELL ioReadFile();
CELL ioWriteFile();
CELL ioCloseFile();
CELL ioGetFilePosition();
CELL ioSetFilePosition();
CELL ioGetFileSize();
CELL ioDeleteFile();
void ioFlushFile();
void update_rx();
void execute(int cell);
void evaluate(char *s);
int not_eol(int ch);
void read_token(FILE *file, char *token_buffer, int echo);
char *read_token_str(char *s, char *token_buffer, int echo);
void ngaGopherUnit();
void ngaFloatingPointUnit();
CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(int opcode);
int ngaValidatePackedOpcodes(CELL opcode);



CELL Dictionary, Compiler;
CELL notfound;

char **sys_argv;
int sys_argc;

/* Some I/O Parameters */

#define MAX_OPEN_FILES   128
#define IO_TTY_PUTC  1000
#define IO_TTY_GETC  1001
#define IO_FS_OPEN    118
#define IO_FS_CLOSE   119
#define IO_FS_READ    120
#define IO_FS_WRITE   121
#define IO_FS_TELL    122
#define IO_FS_SEEK    123
#define IO_FS_SIZE    124
#define IO_FS_DELETE  125
#define IO_FS_FLUSH   126


/* First, a couple of functions to simplify interacting with
   the stack. */

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}

/* Next, functions to translate C strings to/from Retro
   strings. */

int string_inject(char *str, int buffer) {
  int m = strlen(str);
  int i = 0;
  while (m > 0) {
    memory[buffer + i] = (CELL)str[i];
    memory[buffer + i + 1] = 0;
    m--; i++;
  }
  return buffer;
}

char string_data[8192];
char *string_extract(int at) {
  CELL starting = at;
  CELL i = 0;
  while(memory[starting] && i < 8192)
    string_data[i++] = (char)memory[starting++];
  string_data[i] = 0;
  return (char *)string_data;
}

int d_link(CELL dt) {
  return dt + D_OFFSET_LINK;
}

int d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
}

int d_class(CELL dt) {
  return dt + D_OFFSET_CLASS;
}

int d_name(CELL dt) {
  return dt + D_OFFSET_NAME;
}

/* With the dictionary accessors, some functions to actually
   lookup headers. */

int d_lookup(CELL Dictionary, char *name) {
  CELL dt = 0;
  CELL i = Dictionary;
  char *dname;
  while (memory[i] != 0 && i != 0) {
    dname = string_extract(d_name(i));
    if (strcmp(dname, name) == 0) {
      dt = i;
      i = 0;
    } else {
      i = memory[i];
    }
  }
  return dt;
}

CELL d_xt_for(char *Name, CELL Dictionary) {
  return memory[d_xt(d_lookup(Dictionary, Name))];
}

CELL d_class_for(char *Name, CELL Dictionary) {
  return memory[d_class(d_lookup(Dictionary, Name))];
}


/* Now for File I/O functions. These were adapted from the Ngaro VM. */

FILE *ioFileHandles[MAX_OPEN_FILES];

CELL ioGetFileHandle() {
  CELL i;
  for(i = 1; i < MAX_OPEN_FILES; i++)
    if (ioFileHandles[i] == 0)
      return i;
  return 0;
}

CELL ioOpenFile() {
  CELL slot, mode, name;
  slot = ioGetFileHandle();
  mode = data[sp]; sp--;
  name = data[sp]; sp--;
  char *request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  ioFileHandles[slot] = fopen(request, "rb");
    if (mode == 1)  ioFileHandles[slot] = fopen(request, "w");
    if (mode == 2)  ioFileHandles[slot] = fopen(request, "a");
    if (mode == 3)  ioFileHandles[slot] = fopen(request, "rb+");
  }
  if (ioFileHandles[slot] == NULL) {
    ioFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
  return slot;
}

CELL ioReadFile() {
  CELL slot = stack_pop();
  CELL c = fgetc(ioFileHandles[slot]);
  return feof(ioFileHandles[slot]) ? 0 : c;
}

CELL ioWriteFile() {
  CELL slot, c, r;
  slot = data[sp]; sp--;
  c = data[sp]; sp--;
  r = fputc(c, ioFileHandles[slot]);
  return (r == EOF) ? 0 : 1;
}

CELL ioCloseFile() {
  fclose(ioFileHandles[data[sp]]);
  ioFileHandles[data[sp]] = 0;
  sp--;
  return 0;
}

CELL ioGetFilePosition() {
  CELL slot = data[sp]; sp--;
  return (CELL) ftell(ioFileHandles[slot]);
}

CELL ioSetFilePosition() {
  CELL slot, pos, r;
  slot = data[sp]; sp--;
  pos  = data[sp]; sp--;
  r = fseek(ioFileHandles[slot], pos, SEEK_SET);
  return r;
}

CELL ioGetFileSize() {
  CELL slot, current, r, size;
  slot = data[sp]; sp--;
  struct stat buffer;
  int    status;
  status = fstat(fileno(ioFileHandles[slot]), &buffer);
  if (!S_ISDIR(buffer.st_mode)) {
    current = ftell(ioFileHandles[slot]);
    r = fseek(ioFileHandles[slot], 0, SEEK_END);
    size = ftell(ioFileHandles[slot]);
    fseek(ioFileHandles[slot], current, SEEK_SET);
  } else {
    r = -1;
  }
  return (r == 0) ? size : 0;
}

CELL ioDeleteFile() {
  CELL name = data[sp]; sp--;
  char *request = string_extract(name);
  return (unlink(request) == 0) ? -1 : 0;
}

void ioFlushFile() {
  CELL slot;
  slot = data[sp]; sp--;
  fflush(ioFileHandles[slot]);
}

/* Retro needs to track a few variables. This function is
   called as necessary to ensure that the interface stays
   in sync with the image state. */

void update_rx() {
  Dictionary = memory[2];
  Compiler = d_xt_for("Compiler", Dictionary);
  notfound = d_xt_for("err:notfound", Dictionary);
}


/* The `execute` function runs a word in the Retro image.
   It also handles the additional I/O instructions. */

#define UNIX_SYSTEM -8000
#define UNIX_FORK   -8001
#define UNIX_EXIT   -8002
#define UNIX_GETPID -8003
#define UNIX_EXEC0  -8004	
#define UNIX_EXEC1  -8005
#define UNIX_EXEC2  -8006
#define UNIX_EXEC3  -8007
#define UNIX_WAIT   -8008
#define UNIX_KILL   -8009
#define UNIX_POPEN  -8010
#define UNIX_PCLOSE -8011
#define UNIX_WRITE  -8012
#define UNIX_CHDIR  -8013
#define UNIX_GETENV -8014
#define UNIX_PUTENV -8015
#define UNIX_SLEEP  -8016

CELL unixOpenPipe() {
  CELL slot, mode, name;
  slot = ioGetFileHandle();
  mode = data[sp]; sp--;
  name = data[sp]; sp--;
  char *request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  ioFileHandles[slot] = popen(request, "r");
    if (mode == 1)  ioFileHandles[slot] = popen(request, "w");
    if (mode == 3)  ioFileHandles[slot] = popen(request, "r+");
  }
  if (ioFileHandles[slot] == NULL) {
    ioFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
  return slot;
}

CELL unixClosePipe() {
  pclose(ioFileHandles[data[sp]]);
  ioFileHandles[data[sp]] = 0;
  sp--;
  return 0;
}


void execute(int cell) {
  CELL a, b, c;
  CELL opcode;
  char path[1024];
  char arg0[1024], arg1[1024], arg2[1024];
  char arg3[1024], arg4[1024], arg5[1024];
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    if (ip == notfound) {
      printf("%s ?\n", string_extract(TIB));
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else if (opcode >= 0 && opcode < 27) {
      ngaProcessOpcode(opcode);
    } else {
      switch (opcode) {
        case IO_TTY_PUTC:  putc(stack_pop(), stdout); fflush(stdout); break;
        case IO_TTY_GETC:  stack_push(getc(stdin));                   break;
        case IO_FS_OPEN:   ioOpenFile();                              break;
        case IO_FS_CLOSE:  ioCloseFile();                             break;
        case IO_FS_READ:   stack_push(ioReadFile());                  break;
        case IO_FS_WRITE:  ioWriteFile();                             break;
        case IO_FS_TELL:   stack_push(ioGetFilePosition());           break;
        case IO_FS_SEEK:   ioSetFilePosition();                       break;
        case IO_FS_SIZE:   stack_push(ioGetFileSize());               break;
        case IO_FS_DELETE: ioDeleteFile();                            break;
        case IO_FS_FLUSH:  ioFlushFile();                             break;
        case -6000: ngaFloatingPointUnit(); break;
        case -6100: stack_push(sys_argc - 2); break;
        case -6101: a = stack_pop();
                    b = stack_pop();
                    stack_push(string_inject(sys_argv[a + 2], b));
                    break;
        case -6200: ngaGopherUnit(); break;
        case UNIX_SYSTEM: system(string_extract(stack_pop()));        break;
        case UNIX_FORK:   stack_push(fork());                         break;
        case UNIX_EXEC0:  strcpy(path, string_extract(stack_pop()));
                          execl(path, path, (char *)0);
                          stack_push(errno);                          break;
        case UNIX_EXEC1:  strcpy(arg0, string_extract(stack_pop()));
                          strcpy(path, string_extract(stack_pop()));
                          execl(path, path, arg0, (char *)0);
                          stack_push(errno);                          break;
        case UNIX_EXEC2:  strcpy(arg1, string_extract(stack_pop()));
                          strcpy(arg0, string_extract(stack_pop()));
                          strcpy(path, string_extract(stack_pop()));
                          execl(path, path, arg0, arg1, (char *)0);
                          stack_push(errno);                          break;
        case UNIX_EXEC3:  strcpy(arg2, string_extract(stack_pop()));
                          strcpy(arg1, string_extract(stack_pop()));
                          strcpy(arg0, string_extract(stack_pop()));
                          strcpy(path, string_extract(stack_pop()));
                          execl(path, path, arg0, arg1, arg2, (char *)0);
                          stack_push(errno);                          break;
        case UNIX_EXIT:   exit(stack_pop()); break;
        case UNIX_GETPID: stack_push(getpid()); break;
        case UNIX_WAIT:   stack_push(wait(&a)); break;
        case UNIX_KILL:   a = stack_pop();
                          kill(stack_pop(), a);
                          break;
        case UNIX_POPEN:  unixOpenPipe(); break;
        case UNIX_PCLOSE: unixClosePipe(); break;
        case UNIX_WRITE:  c = stack_pop();
                          b = stack_pop();
                          a = stack_pop();
                          write(fileno(ioFileHandles[c]), string_extract(a), b);
                          break;
        case UNIX_CHDIR:  chdir(string_extract(stack_pop()));
                          break;
        case UNIX_GETENV: a = stack_pop();
                          b = stack_pop();
                          string_inject(getenv(string_extract(b)), a);
                          break;
        case UNIX_PUTENV: putenv(string_extract(stack_pop()));
                          break;
        case UNIX_SLEEP:  sleep(stack_pop());
                          break;
        default:   printf("Invalid instruction!\n");
                   printf("At %d, opcode %d\n", ip, opcode);
                   exit(1);
      }
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}


/* The `evaluate` function moves a token into the Retro
   token buffer, then calls the Retro `interpret` word
   to process it. */

void evaluate(char *s) {
  if (strlen(s) == 0)
    return;
  update_rx();
  CELL interpret = d_xt_for("interpret", Dictionary);
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret);
}


/* `read_token` reads a token from the specified file.
   It will stop on a whitespace or newline. It also
   tries to handle backspaces, though the success of this
   depends on how your terminal is configured. */

int not_eol(int ch) {
  return (ch != (char)10) && (ch != (char)13) && (ch != (char)32) && (ch != EOF) && (ch != 0);
}

void read_token(FILE *file, char *token_buffer, int echo) {
  int ch = getc(file);
  if (echo != 0)
    putchar(ch);
  int count = 0;
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = getc(file);
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
}

char *read_token_str(char *s, char *token_buffer, int echo) {
  int ch = (char)*s++;
  if (echo != 0)
    putchar(ch);
  int count = 0;
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = (char)*s++;
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
  return s;
}

double Floats[8192];
CELL fsp;

void float_push(double value) {
    fsp++;
    Floats[fsp] = value;
}

double float_pop() {
    fsp--;
    return Floats[fsp + 1];
}

void float_from_number() {
    float_push((double)stack_pop());
}

void float_from_string() {
    float_push(atof(string_extract(stack_pop())));
}

void float_to_string() {
    snprintf(string_data, 8192, "%f", float_pop());
    string_inject(string_data, stack_pop());
}

void float_add() {
    double a = float_pop();
    double b = float_pop();
    float_push(a+b);
}

void float_sub() {
    double a = float_pop();
    double b = float_pop();
    float_push(b-a);
}

void float_mul() {
    double a = float_pop();
    double b = float_pop();
    float_push(a*b);
}

void float_div() {
    double a = float_pop();
    double b = float_pop();
    float_push(b/a);
}

void float_floor() {
    float_push(floor(float_pop()));
}

void float_eq() {
    double a = float_pop();
    double b = float_pop();
    if (a == b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_neq() {
    double a = float_pop();
    double b = float_pop();
    if (a != b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_lt() {
    double a = float_pop();
    double b = float_pop();
    if (b < a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_gt() {
    double a = float_pop();
    double b = float_pop();
    if (b > a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_depth() {
    stack_push(fsp);
}

void float_dup() {
    double a = float_pop();
    float_push(a);
    float_push(a);
}

void float_drop() {
    float_pop();
}

void float_swap() {
    double a = float_pop();
    double b = float_pop();
    float_push(a);
    float_push(b);
}

void float_log() {
    double a = float_pop();
    double b = float_pop();
    float_push(log(b) / log(a));
}

void float_pow() {
    double a = float_pop();
    double b = float_pop();
    float_push(pow(b, a));
}

void float_to_number() {
    double a = float_pop();
    if (a > 2147483647)
      a = 2147483647;
    if (a < -2147483648)
      a = -2147483648;
    stack_push((CELL)round(a));
}

void float_sin() {
  float_push(sin(float_pop()));
}

void float_cos() {
  float_push(cos(float_pop()));
}

void float_tan() {
  float_push(tan(float_pop()));
}

void float_asin() {
  float_push(asin(float_pop()));
}

void float_acos() {
  float_push(acos(float_pop()));
}

void float_atan() {
  float_push(atan(float_pop()));
}

void ngaFloatingPointUnit() {
    switch (stack_pop()) {
        case 0: float_from_number();  break;
        case 1: float_from_string();  break;
        case 2: float_to_string();    break;
        case 3: float_add();          break;
        case 4: float_sub();          break;
        case 5: float_mul();          break;
        case 6: float_div();          break;
        case 7: float_floor();        break;
        case 8: float_eq();           break;
        case 9: float_neq();          break;
        case 10: float_lt();          break;
        case 11: float_gt();          break;
        case 12: float_depth();       break;
        case 13: float_dup();         break;
        case 14: float_drop();        break;
        case 15: float_swap();        break;
        case 16: float_log();         break;
        case 17: float_pow();         break;
        case 18: float_to_number();   break;
        case 19: float_sin();         break;
        case 20: float_cos();         break;
        case 21: float_tan();         break;
        case 22: float_asin();        break;
        case 23: float_acos();        break;
        case 24: float_atan();        break;
        default:                      break;
    }
}

void error(const char *msg) {
  perror(msg);
  exit(0);
}

void gopher_fetch(char *host, CELL port, char *selector, CELL dest) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char data[128 * 1024 + 1];
  char buffer[1025];
  portno = (int)port;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  server = gethostbyname(host);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  bzero(data, 128 * 1024 + 1);
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
     (char *)&serv_addr.sin_addr.s_addr,
     server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR connecting");
  n = write(sockfd,selector,strlen(selector));
  if (n < 0)
     error("ERROR writing to socket");
  n = write(sockfd,"\n",strlen("\n"));
  if (n < 0)
     error("ERROR writing to socket");
  n = 1;
  while (n > 0) {
    bzero(buffer,1025);
    n = read(sockfd,buffer,1024);
    strcat(data, buffer);
  }
  close(sockfd);
  string_inject(data, dest);
  stack_push(strlen(data));
}


/* <addr> <server> <port> <selector> */
void ngaGopherUnit() {
  char server[1025];
  char selector[4097];
  CELL port, dest;
  strcpy(selector, string_extract(stack_pop()));
  port = stack_pop();
  strcpy(server, string_extract(stack_pop()));
  dest = stack_pop();
  gopher_fetch(server, port, selector, dest);
}

/* Compile image.c and link against the image.o */
#include "image.c"

void dump_stack() {
  CELL i;
  if (sp == 0)
    return;
  printf("\nStack: ");
  for (i = 1; i <= sp; i++) {
    if (i == sp)
      printf("[ TOS: %d ]", data[i]);
    else
      printf("%d ", data[i]);
  }
  printf("\n");
}


int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 1;
  if (b == 0) return 1;
              return 0;
}


void include_file(char *fname) {
  int inBlock = 0;
  char source[64 * 1024];
  char fence[4];
  FILE *fp;
  fp = fopen(fname, "r");
  if (fp == NULL)
    return;
  while (!feof(fp))
  {
    read_token(fp, source, 0);
    strncpy(fence, source, 3);
    fence[3] = '\0';
    if (fenced(fence)) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1)
        evaluate(source);
    }
  }
  fclose(fp);
}


int main(int argc, char **argv) {
  int i;
  ngaPrepare();
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
  update_rx();

  sys_argc = argc;
  sys_argv = argv;

  if (argc > 1) {
    if (strcmp(argv[1], "-i") == 0) {
      execute(d_xt_for("listen", Dictionary));
    } else {
      include_file(argv[1]);
    }
  }

  if (sp >= 1)
    dump_stack();

  exit(0);
}

/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2017, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  else {
    printf("Unable to find the ngaImage!\n");
    exit(1);
  }
  return imageSize;
}

void ngaPrepare() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = VM_NOP;
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}

void inst_nop() {
}

void inst_lit() {
  sp++;
  ip++;
  TOS = memory[ip];
}

void inst_dup() {
  sp++;
  data[sp] = NOS;
}

void inst_drop() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_swap() {
  int a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_push() {
  rp++;
  TORS = TOS;
  inst_drop();
}

void inst_pop() {
  sp++;
  TOS = TORS;
  rp--;
}

void inst_jump() {
  ip = TOS - 1;
  inst_drop();
}

void inst_call() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_drop();
}

void inst_ccall() {
  int a, b;
  a = TOS; inst_drop();  /* False */
  b = TOS; inst_drop();  /* Flag  */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

void inst_return() {
  ip = TORS;
  rp--;
}

void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_drop();
}

void inst_neq() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_drop();
}

void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_drop();
}

void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_drop();
}

void inst_fetch() {
  switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    default: TOS = memory[TOS]; break;
  }
}

void inst_store() {
  memory[TOS] = NOS;
  inst_drop();
  inst_drop();
}

void inst_add() {
  NOS += TOS;
  inst_drop();
}

void inst_sub() {
  NOS -= TOS;
  inst_drop();
}

void inst_mul() {
  NOS *= TOS;
  inst_drop();
}

void inst_divmod() {
  int a, b;
  a = TOS;
  b = NOS;
  TOS = b / a;
  NOS = b % a;
}

void inst_and() {
  NOS = TOS & NOS;
  inst_drop();
}

void inst_or() {
  NOS = TOS | NOS;
  inst_drop();
}

void inst_xor() {
  NOS = TOS ^ NOS;
  inst_drop();
}

void inst_shift() {
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (TOS * -1);
  else {
    if (x < 0 && y > 0)
      NOS = x >> y | ~(~0U >> y);
    else
      NOS = x >> y;
  }
  inst_drop();
}

void inst_zret() {
  if (TOS == 0) {
    inst_drop();
    ip = TORS;
    rp--;
  }
}

void inst_end() {
  ip = IMAGE_SIZE;
}

typedef void (*Handler)(void);
Handler instructions[NUM_OPS] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_end
};

void ngaProcessOpcode(CELL opcode) {
  instructions[opcode]();
}

int ngaValidatePackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  CELL current;
  int valid = -1;
  int i;
  for (i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 26))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void ngaProcessPackedOpcodes(int opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    ngaProcessOpcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
