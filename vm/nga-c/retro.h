/**************************************************************
  Shared declarations for the RETRO C runtime.

  This is included by every separately compiled runtime source. The
  amalgamated build includes it once through retro.c.
**************************************************************/

#ifndef RETRO_RUNTIME_H
#define RETRO_RUNTIME_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "devices.h"
#include "nga_core.h"

#ifndef NO_EMBEDDED_IMAGE
extern CELL ngaImageCells;
extern CELL ngaImage[];
#endif

/* Device entry points are generated from the shared device manifest. */
#define DEVICE(name) V io_ ## name(NgaState *); V query_ ## name(NgaState *);
#define DEVICE_WITH_INIT(name, init) DEVICE(name) V init(NgaState *);
#include "devices.def"
#undef DEVICE_WITH_INIT
#undef DEVICE

#ifdef ENABLE_FLOATS
V float_push(NgaState *, double);
double float_pop(NgaState *);
#endif

#ifdef ENABLE_FILES
CELL files_get_handle(NgaState *);
FILE *files_get_open_handle(NgaState *, CELL, const char *);
#endif

#endif
