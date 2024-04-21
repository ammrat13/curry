#include "curry.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

// Size of the buffer we allocate for the curried function. It has to be large
// enough to house all the generated code given how many arguments we can take.
//
// Currently, we just hope this is enough. A rigorous assessment will be done
// after we know what code we're generating.
#define CURRY_BUF_SIZE (4096)

void *curry(void *fn, size_t nargs_now, size_t nargs_later, ...) {
  // Create the `va_list` to forward to `vcurry`
  va_list args_now;
  va_start(args_now, nargs_later);
  // Do the call
  void *const ret = vcurry(fn, nargs_now, nargs_later, args_now);
  // Remember to clean up before
  va_end(args_now);
  return ret;
}

// This function does the actual work of constructing the returned function. It
// assumes the buffer is already allocated as writeable and that it's
// sufficiently large.
static void vcurry_write_thunk(uint8_t *buf, void *fn, size_t nargs_now,
                               size_t nargs_later, va_list args_now);

void *vcurry(void *fn, size_t nargs_now, size_t nargs_later, va_list args_now) {

  // Limit the total number of arguments, both for now-args and later-args. The
  // number of arguments affects the length of the generated code.
  if (nargs_now > CURRY_NARGS_MAX || nargs_later > CURRY_NARGS_MAX)
    return NULL;

  // We can have zero now-args and zero later-args. If we have no now-args, we
  // can just return the supplied function since it already does what we want.
  // If we have no later-args is zero, we will return a thunk that will call
  // `fn` with the arguments we've already received.
  if (nargs_now == 0)
    return fn;

  // Allocate a buffer to store the generated code. This has to be done with
  // `mmap` since we will be changing its permissions later.
  uint8_t *const ret = mmap(NULL, CURRY_BUF_SIZE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == MAP_FAILED)
    return NULL;

  // Actually construct the thunk
  vcurry_write_thunk(ret, fn, nargs_now, nargs_later, args_now);

  // Make the buffer executable, and return it. On failure, remember to free the
  // buffer.
  if (mprotect(ret, CURRY_BUF_SIZE, PROT_READ | PROT_EXEC) == -1) {
    munmap(ret, CURRY_BUF_SIZE);
    return NULL;
  }
  return ret;
}

static void vcurry_write_thunk(uint8_t *buf, void *fn, size_t nargs_now,
                               size_t nargs_later, va_list args_now) {
  // Compute how many arguments we have in total
  const size_t nargs_total = nargs_now + nargs_later;
  // Create a pointer we'll use to iterate over the buffer
  uint8_t *cur = buf;

  // It's possible that we're running with CET enabled. Like GCC, we need to
  // emit an `endbr64` as our first instruction.
  {
    *cur++ = 0xf3u;
    *cur++ = 0x0fu;
    *cur++ = 0x1eu;
    *cur++ = 0xfau;
  }

  // Allocate space for all the overflow-args.
  {
    // enter $(8 * nargs_overflow), 0
    const size_t nargs_overflow = nargs_total > 6 ? nargs_total - 6 : 0;
    const size_t bytes_overflow = 8 * nargs_overflow;
    *cur++ = 0xc8u;
    *cur++ = (bytes_overflow >> 0u) & 0xffu;
    *cur++ = (bytes_overflow >> 8u) & 0xffu;
    *cur++ = 0x00u;
  }

  (void)fn;
  (void)args_now;
}
