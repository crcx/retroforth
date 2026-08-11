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

#include "retro.h"

#ifdef ENABLE_RNG
#if defined(__linux__)
#include <sys/syscall.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

static int rng_bytes_from_urandom(unsigned char *buffer, size_t size) {
  size_t remaining = size;
  unsigned char *at = buffer;
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    return 0;
  }
  while (remaining > 0) {
    ssize_t count = read(fd, at, remaining);
    if (count > 0) {
      at += count;
      remaining -= (size_t)count;
    } else if (count == -1 && errno == EINTR) {
      continue;
    } else {
      close(fd);
      return 0;
    }
  }
  if (close(fd) == -1) {
    return 0;
  }
  return 1;
}

static int rng_bytes(unsigned char *buffer, size_t size) {
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  arc4random_buf(buffer, size);
  return 1;
#elif defined(_WIN32) || defined(_WIN64)
  typedef LONG (WINAPI *RngFunction)(void *, unsigned char *, ULONG, ULONG);
  HMODULE library = LoadLibraryA("bcrypt.dll");
  RngFunction function;
  LONG status;
  if (library == NULL) return 0;
  function = (RngFunction)GetProcAddress(library, "BCryptGenRandom");
  if (function == NULL) {
    FreeLibrary(library);
    return 0;
  }
  status = function(NULL, buffer, (ULONG)size, 0x00000002);
  FreeLibrary(library);
  return status == 0;
#else
#if defined(__linux__) && defined(SYS_getrandom)
  size_t remaining = size;
  unsigned char *at = buffer;
  while (remaining > 0) {
    ssize_t count = syscall(SYS_getrandom, at, remaining, 0);
    if (count > 0) {
      at += count;
      remaining -= (size_t)count;
    } else if (count == -1 && errno == EINTR) {
      continue;
    } else if (count == -1 && errno == ENOSYS) {
      break;
    } else {
      return 0;
    }
  }
  if (remaining == 0) return 1;
#endif
  return rng_bytes_from_urandom(buffer, size);
#endif
}

V io_rng(NgaState *vm) {
  uint64_t r = 0;
  unsigned char buffer[8];
  int i;
  if (!rng_bytes(buffer, sizeof(buffer))) {
    stack_push(vm, 0);
    return;
  }
  for(i = 0; i < 8; ++i) {
    r = r << 8;
    r += (uint64_t)buffer[i];
  }
#ifndef BIT64
  stack_push(vm, (CELL)(r & INT32_MAX));
#else
  stack_push(vm, (CELL)(r & INT64_MAX));
#endif
}

V query_rng(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_RNG);
}
#endif
