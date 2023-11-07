/* nga.c (c) Charles Childers, Luke Parrish, Marc Simpson
             Jay Skeer, Kenneth Keating                       */

/* cc -o nga-11 vm/nga-c/nga-x11.c `pkg-config --cflags x11 --libs` */

/* Local Configuration */

#define ROM     "ngaImage"
#define FONT    "retro.fnt"
#define FG      0xFFFFFF
#define BG      0x000055
#define CURSOR  0x00FFFF
#define FW      640
#define FH      480
#define FONT_H  16
#define FONT_W  8

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define V void
#define I int
#define C char

#define BIT64

#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#ifndef IMAGE_SIZE
#define IMAGE_SIZE   65536       /* Amount of RAM, in cells */
#endif

#ifndef ADDRESSES
#define ADDRESSES    256          /* Depth of address stack */
#endif

#ifndef DEPTH
#define DEPTH  256          /* Depth of data stack */
#endif

CELL sp, rp, ip;                  /* Stack and instruction pointers */
CELL data[DEPTH];           /* The data stack          */
CELL address[ADDRESSES];          /* The address stack       */
CELL m[IMAGE_SIZE + 1];      /* Image m            */

#define T  data[sp]             /* Top item on stack       */
#define N  data[sp-1]           /* Second item on stack    */
#define TORS address[rp]          /* Top item on address stack */

typedef V (*Handler)(V);

CELL pop();
V push(CELL value);
V execute(CELL cell);
V load_image(char *imageFile);
V prepare_vm();
V process_opcode_bundle(CELL opcode);
V dputc(I);
V redraw();

#define TERM_W (FW / FONT_W)
#define TERM_H (FH / FONT_H)

I frame[(FW * FH)/32];    /* frame buffer for display */
I font[8192];             /* font bitmap              */
I tx, ty;                 /* text cursor location     */

/* variables for Arland's DEC subset                  */
I acc[32], acc_i = 0, ground = 0, on = 0;

/* variables for X11 stuff                            */
Display *disp;
Window win;
GC gc;
XEvent event;
C text[255];


V cursor() {
  XSetForeground(disp, gc, CURSOR);
  XFillRectangle(disp, win, gc, tx, ty, FONT_W, FONT_H);
  XFlush(disp);
}

I wait_key() {
    while (1) {
        XNextEvent(disp, &event);
        if (event.type == KeyPress) {
            KeySym key = XLookupKeysym(&event.xkey, 0);
            XLookupString(&event.xkey, text, 255, &key, 0);
            if (text[0] == 13) text[0] = 10;
            if (text[0] != 0)  return (I) text[0];
        }
        if (event.type == Expose) { redraw(); }
    }
}

V pixel(I x, I y, I c) {
  I index = y * FW + x;
  if (c == 0) {
    frame[(index / 32)] &= ~(1 << (31 - index % 32));
    XSetForeground(disp, gc, BG);
  } else {
    frame[(index / 32)] |= 1 << (31 - index % 32);
    XSetForeground(disp, gc, FG);
  }
  XDrawPoint(disp, win, gc, x, y);
}

I get_pixel(I x, I y) {
  I index = y * FW + x;
  I bit_pos = 31 - index % 32;
  I mask = 1 << bit_pos;
  return (frame[(index / 32)] & mask) >> bit_pos;
}

V redraw() {
  for (I x = 0; x < FW; x++) {
    for (I y = 0; y < FH; y++) {
      XSetForeground(disp, gc, get_pixel(x, y) ? FG : BG);
      XDrawPoint(disp, win, gc, x, y);
    }
  }
  cursor();
}

V scroll() {
  I memsize = (FH * FW) / 32;
  I last_row = (FW * FONT_H) / 32;
  for (I i = 0; i < memsize - last_row; i++) {
    frame[i] = frame[i + last_row];
  }
  for (I i = memsize - last_row; i < memsize; i++) {
    frame[i] = 0;
  }
  redraw();
}

unsigned C reverse(unsigned C b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

V handle_special_character(I c) {
  if (c == 8 || c == 127) { /* BACKSPACE/DELETE */
    if (tx - FONT_W >= 0) {
      tx -= FONT_W ;
      dputc(' ');
      tx -= FONT_W;
      redraw();
    }
  } else if (c == 9) { /* TAB */
    I ts = tx / FONT_W;
    for (I i = ts; i < ((ts + 7) / 8) * 8; i++) { dputc(' '); }
  } else if (c == 10 || c == 13) { /* CR or LF */
    ty += FONT_H;
    tx = 0;
    redraw();
  }
}

V handle_regular_character(C *bitmap) {
  I x, y, set;
  for (x = 0; x < FONT_H; x++) {
    for (y = 0; y < FONT_W; y++) {
      set = reverse(bitmap[x]) & 1 << y;
      pixel(tx + y, ty + x, set ? 1 : 0);
    }
  }
  tx += FONT_W;
}

V advance_cursor_and_scroll() {
  if (tx >= FW) { tx = 0; ty += FONT_H; }
  if (ty > (FH - FONT_H)) { ty -= FONT_H; scroll(); }
}

V dputc(I c) {
  C *bitmap = (C *)font + (c * ((FONT_H * FONT_W) / 8));

  handle_special_character(c);

  if (!(c == 8 || c == 127 || c == 9 || c == 10 || c == 13)) {
    handle_regular_character(bitmap);
  }

  advance_cursor_and_scroll();
  cursor();
}

I acc_pop() {
  I k = acc_i;
  if (acc_i > 0) {
    acc_i -= 1;
    if (acc_i == -1) {
      acc_i = 0;
    }
  }
  return acc[k];
}

V acc_reset() {
  for (I x = 0; x < 32; x++) { acc[x] = 0; }
  ground = on = 0;
}

V gt_up(I n)       { ty -= FONT_H; if (ty < 0) ty = 0;  }
V gt_down(I n)     { ty += FONT_H; if (ty > 24 * FONT_H) ty = 24 * FONT_H; }
V gt_left(I n)     { tx -= FONT_W; if (tx < 0) tx = 0; }
V gt_right(I n)    { tx += FONT_W; if (tx > 79 * FONT_W) ty = 79 * FONT_W; }
V gt_move_cursor() {
  if (acc_i >= 1) {
    I a = (acc_pop() - 1) * FONT_W;
    I b = (acc_pop() - 1) * FONT_H;
    ty = b; if (ty < 0) ty = 0; if (ty > FONT_H * 24) ty = FONT_H * 24;
    tx = a; if (tx < 0) tx = 0; if (tx > FONT_W * 79) tx = FONT_W * 79;
    acc_reset();
    return;
  }
  /* home */
  tx = 0; ty = 0;
  acc_reset();
}

V gt_reset()       { acc_reset(); }
V gt_next()        { acc_i += 1; acc[acc_i] = 0; }
V gt_clear() {
  acc_reset();
  for (I x = 0; x < (FW * FH) / 32; x++) frame[x] = 0;
  tx = ty = 0;
  redraw();
}
V gt_set_attr()    { acc_reset(); }

V term_putc(I c) {
  if (c == 27) {
    on = -2;
    return;
  }
  switch (on) {
    case 0:
      dputc(c);
      return;
    case -2:
      if (c == '[') {
        on = -1;
        return;
      }
      on = 0;
      dputc(c);
      return;
  }
  if (c >= '0' && c <= '9') {
    acc[acc_i] *= 10;
    acc[acc_i] += c - 48;
    if (c == '3') {
      ground = 0;
    }
    if (c == '4') {
      ground = 1;
    }
  } else {
    switch (c) {
      case 'A': gt_up(acc_pop());    break;
      case 'B': gt_down(acc_pop());  break;
      case 'C': gt_left(acc_pop());  break;
      case 'D': gt_right(acc_pop()); break;
      case 'm': gt_set_attr();       break;
      case ';': gt_next();           break;
      case 'H': gt_move_cursor();    break;
      case 'J': gt_clear();          break;
      default: on = 0;
               dputc(c);
    }
  }
}

V load_font() {
  I f = open(FONT, O_RDONLY, 0666);
  if (!f) { return; };
  read(f, &font, 4096);
  close(f);
}

CELL pop() { sp--; return data[sp + 1]; }
V push(CELL value) { sp++; data[sp] = value; }

V execute(CELL cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    opcode = m[ip];
    process_opcode_bundle(opcode);
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}

I main(I argc, char **argv) {
  prepare_vm();
  load_image("ngaImage");
  load_font();

  disp = XOpenDisplay(NULL);
  win = XCreateSimpleWindow(disp, RootWindow(disp, 0),
                            0, 0, FW, FH, 0, 0x0, 0x0);
  XSelectInput(disp, win, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
  XStoreName(disp, win, "x/ilo");
  gc = DefaultGC(disp, DefaultScreen(disp));
  XMapWindow(disp, win);
  XFlush(disp);
  XMoveWindow(disp, win, (DisplayWidth(disp, DefaultScreen(disp)) - FW) / 2,
                         (DisplayHeight(disp, DefaultScreen(disp)) - FH) / 2);

  execute(0);

  XUnmapWindow(disp, win);
  XDestroyWindow(disp, win);
  XCloseDisplay(disp);
  exit(0);
}

V load_image(char *imageFile) {
  FILE *fp;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    fread(&m, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
}

V prepare_vm() {
  for (ip = 0; ip < IMAGE_SIZE; ip++) m[ip] = 0;
  for (ip = 0; ip < DEPTH; ip++)      data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)  address[ip] = 0;
  ip = sp = rp = 0;
}

V inst_no() { }
V inst_li() { ip++; push(m[ip]); }
V inst_du() { push(T); }
V inst_dr() { pop(); }
V inst_sw() { CELL a= T; T = N; N = a; }
V inst_pu() { rp++; TORS = T; inst_dr(); }
V inst_po() { push(TORS); rp--; }
V inst_ju() { ip = T - 1; inst_dr(); }
V inst_ca() { rp++; TORS = ip; ip = T - 1; inst_dr(); }
V inst_cc() {
  CELL a, b;
  a = T; inst_dr();  /* Target */
  b = T; inst_dr();  /* Flag   */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

V inst_re() { ip = TORS; rp--; }
V inst_eq() { N = (N == T) ? -1 : 0; inst_dr(); }
V inst_ne() { N = (N != T) ? -1 : 0; inst_dr(); }
V inst_lt() { N = (N < T)  ? -1 : 0; inst_dr(); }
V inst_gt() { N = (N > T)  ? -1 : 0; inst_dr(); }

V inst_fe() {
  switch (T) {
    case -1: T = sp - 1; break;
    case -2: T = rp; break;
    case -3: T = IMAGE_SIZE; break;
    case -4: T = CELL_MIN; break;
    case -5: T = CELL_MAX; break;
    default: T = m[T]; break;
  }
}

V inst_st() {
  if (T <= IMAGE_SIZE && T >= 0) { m[T] = N; pop(); pop(); }
  else { ip = IMAGE_SIZE; }
}

V inst_ad() { N += T; inst_dr(); }
V inst_su() { N -= T; inst_dr(); }
V inst_mu() { N *= T; inst_dr(); }
V inst_di() { CELL a, b; a = T; b = N; T = b / a; N = b % a; }
V inst_an() { N = T & N; inst_dr(); }
V inst_or() { N = T | N; inst_dr(); }
V inst_xo() { N = T ^ N; inst_dr(); }

V inst_sh() {
  CELL y = T;
  CELL x = N;
  if (T < 0)
    N = N << (0 - T);
  else {
    if (x < 0 && y > 0)
      N = x >> y | ~(~0U >> y);
    else
      N = x >> y;
  }
  inst_dr();
}

V inst_zr() { if (T == 0) { pop(); ip = TORS; rp--; } }
V inst_ha() { ip = IMAGE_SIZE; exit(0); }
V inst_ie() { push(5); }

V inst_iq() {
  switch (pop()) {
    case 0:
      push(0);
      push(0);
      break;
    case 1:
      push(1);
      push(1);
      break;
    case 2:
      push(0);
      push(33);
      break;
    case 3:
      push(0);
      push(34);
      break;
    case 4:
      push(0);
      push(35);
      break;
  }
}

V inst_ii() {
  I c, x, y, wx, wy, mask; Window w;
  switch (pop()) {
    case 0:
      dputc((I)pop());
      break;
    case 1:
      c = wait_key();
      dputc((I)c);
      push(c);
      break;
    case 2:
      c = pop();
      y = pop();
      x = pop();
      pixel(x, y, c);
      break;
    case 3:
      y = pop();
      x = pop();
      c = get_pixel(x, y);
      push(c);
      break;
    case 4:
      XQueryPointer(disp, win, &w, &w, &x, &y, &wx, &wy, &mask);
      push(wx); push(wy);
      if (mask & Button1Mask) push(-1); else push(0);
      break;
  }
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

V process_opcode_bundle(CELL opcode) {
  instructions[opcode & 0xFF]();
  instructions[(opcode >> 8) & 0xFF]();
  instructions[(opcode >> 16) & 0xFF]();
  instructions[(opcode >> 24) & 0xFF]();
}
