#include "curry.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

// Size of the buffer we allocate for the curried function. It has to be large
// enough to house all the generated code given how many arguments we can take.
//
// Currently, we just hope this is enough. A rigorous assessment will be done
// after we know what code we're generating.
#define CURRY_BUF_SIZE (8192)

void *curry(void *fn, size_t nargs_now, size_t nargs_later, ...) {

  // Limit the total number of arguments. The number of arguments affects the
  // length of the generated code.
  if (nargs_now > CURRY_NARGS_MAX || nargs_later > CURRY_NARGS_MAX)
    return NULL;

  // Both `nargs_now` and `nargs_later` can be zero. If `nargs_now` is zero, we
  // can just return the supplied function since it already does what we want.
  // If `nargs_later` is zero, we will return a thunk that will call `fn` with
  // the arguments we've already received.
  if (nargs_now == 0)
    return fn;

  // Allocate a buffer to store the generated code. This has to be done with
  // `mmap` since we will be changing its permissions later.
  uint8_t *const ret = mmap(NULL, CURRY_BUF_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == MAP_FAILED)
    return NULL;
  // Create a pointer to where we're currently writing in the return buffer.
  uint8_t *cur = ret;

  return ret;
}
