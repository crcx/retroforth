/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2020, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef NUM_DEVICES
#define NUM_DEVICES 0
#endif

CELL load_image(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  CELL i;
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
    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  else {
    for (i = 0; i < ngaImageCells; i++)
      memory[i] = ngaImage[i];
    imageSize = i;
  }
  return imageSize;
}

void prepare_vm() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = 0; /* NO - nop instruction */
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}

void inst_no() {
}

void inst_li() {
  sp++;
  ip++;
  TOS = memory[ip];
}

void inst_du() {
  sp++;
  data[sp] = NOS;
}

void inst_dr() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_sw() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_pu() {
  rp++;
  TORS = TOS;
  inst_dr();
}

void inst_po() {
  sp++;
  TOS = TORS;
  rp--;
}

void inst_ju() {
  ip = TOS - 1;
  inst_dr();
}

void inst_ca() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_dr();
}

void inst_cc() {
  CELL a, b;
  a = TOS; inst_dr();  /* Target */
  b = TOS; inst_dr();  /* Flag   */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

void inst_re() {
  ip = TORS;
  rp--;
}

void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_dr();
}

void inst_ne() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_dr();
}

void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_dr();
}

void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_dr();
}

void inst_fe() {
#ifndef NOCHECKS
  if (TOS >= IMAGE_SIZE || TOS < -5) {
    ip = IMAGE_SIZE;
    printf("\nERROR (nga/inst_fe): Fetch beyond valid memory range\n");
    exit(1);
  } else {
#endif
    switch (TOS) {
      case -1: TOS = sp - 1; break;
      case -2: TOS = rp; break;
      case -3: TOS = IMAGE_SIZE; break;
      case -4: TOS = CELL_MIN; break;
      case -5: TOS = CELL_MAX; break;
      default: TOS = memory[TOS]; break;
    }
#ifndef NOCHECKS
  }
#endif
}

void inst_st() {
#ifndef NOCHECKS
  if (TOS <= IMAGE_SIZE && TOS >= 0) {
#endif
    memory[TOS] = NOS;
    inst_dr();
    inst_dr();
#ifndef NOCHECKS
  } else {
    ip = IMAGE_SIZE;
    printf("\nERROR (nga/inst_st): Store beyond valid memory range\n");
    exit(1);
  }
#endif
}

void inst_ad() {
  NOS += TOS;
  inst_dr();
}

void inst_su() {
  NOS -= TOS;
  inst_dr();
}

void inst_mu() {
  NOS *= TOS;
  inst_dr();
}

void inst_di() {
  CELL a, b;
  a = TOS;
  b = NOS;
#ifndef NOCHECKS
  if (a == 0) {
    printf("\nERROR (nga/inst_di): Division by zero\n");
    exit(1);
  }
#endif
  TOS = b / a;
  NOS = b % a;
}

void inst_an() {
  NOS = TOS & NOS;
  inst_dr();
}

void inst_or() {
  NOS = TOS | NOS;
  inst_dr();
}

void inst_xo() {
  NOS = TOS ^ NOS;
  inst_dr();
}

void inst_sh() {
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
  inst_dr();
}

void inst_zr() {
  if (TOS == 0) {
    inst_dr();
    ip = TORS;
    rp--;
  }
}

void inst_ha() {
  ip = IMAGE_SIZE;
}

void inst_ie() {
  sp++;
  TOS = NUM_DEVICES;
}

void inst_iq() {
  CELL Device = TOS;
  inst_dr();
  IO_queryHandlers[Device]();
}

void inst_ii() {
  CELL Device = TOS;
  inst_dr();
  IO_deviceHandlers[Device]();
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

void process_opcode(CELL opcode) {
  if (opcode != 0)
    instructions[opcode]();
}

int validate_opcode_bundle(CELL opcode) {
  CELL raw = opcode;
  CELL current;
  int valid = -1;
  int i;
  for (i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 29))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void process_opcode_bundle(CELL opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    process_opcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
