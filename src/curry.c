#include "curry.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

// Utility functions for determining the size of an argument
static bool is_u32(uint64_t x) {
  return (x & UINT64_C(0xffffffff00000000)) == 0;
};
static bool is_i32(uint64_t x) {
  const uint64_t t = x & UINT64_C(0xffffffff80000000);
  return t == 0 || t == UINT64_C(0xffffffff80000000);
};

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
static void vcurry_write_thunk(uint8_t *buf, size_t buf_size, void *fn,
                               size_t nargs_now, size_t nargs_later,
                               va_list args_now);
// Compute an upper bound on the number of bytes that `vcurry_write_thunk` will
// write. This has to be kept in sync with that function. TODO: Improve this
// estimate - in theory we could duplicate the code and get an exact number.
static size_t vcurry_estimate_thunk_size(size_t nargs_now, size_t nargs_later);
// Where we return to from `munmap`. It's actually some code.
extern uint8_t vcurry_return_trampoline;

void *vcurry(void *fn, size_t nargs_now, size_t nargs_later, va_list args_now) {

  // We can have zero now-args and zero later-args. If we have no now-args, we
  // can just return the supplied function since it already does what we want.
  // If we have no later-args is zero, we will return a thunk that will call
  // `fn` with the arguments we've already received.
  if (nargs_now == 0)
    return fn;

  // Allocate a buffer to store the generated code. This has to be done with
  // `mmap` since we will be changing its permissions later.
  const size_t ret_size = vcurry_estimate_thunk_size(nargs_now, nargs_later);
  uint8_t *const ret = mmap(NULL, ret_size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == MAP_FAILED)
    return NULL;

  // Actually construct the thunk
  vcurry_write_thunk(ret, ret_size, fn, nargs_now, nargs_later, args_now);

  // Make the buffer executable, and return it. On failure, remember to free the
  // buffer.
  if (mprotect(ret, ret_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(ret, ret_size);
    return NULL;
  }
  return ret;
}

// Identifiers for registers. We don't include all of them - just the ones used
// for arguments, temporary registers we can clobber, and stack management
// registers.
typedef enum reg_id_t {
  REG_ID_RAX = 0,
  REG_ID_RCX = 1,
  REG_ID_RDX = 2,
  REG_ID_RSP = 4,
  REG_ID_RBP = 5,
  REG_ID_RSI = 6,
  REG_ID_RDI = 7,
  REG_ID_R8 = 8,
  REG_ID_R9 = 9,
  REG_ID_R10 = 10,
  REG_ID_R11 = 11,
} reg_id_t;

// Convert an argument index to a register identifier
static reg_id_t argidx_to_regid(size_t argidx);

// Emit: mov $(dst), $(src)
static uint8_t *emit_mov_reg_reg(uint8_t *cur, reg_id_t dst, reg_id_t src);
// Emit: mov [%rsp + $(8 * off)], $(src)
static uint8_t *emit_mov_rsp_reg(uint8_t *cur, size_t slot, reg_id_t src);
// Emit: mov $(dst), [%rbp + $(8 * off + 16)]
static uint8_t *emit_mov_reg_rbp(uint8_t *cur, reg_id_t dst, size_t slot);
// Emit: mov $(dst), $(imm)
static uint8_t *emit_mov_reg_imm(uint8_t *cur, reg_id_t dst, uint64_t imm);

static void vcurry_write_thunk(uint8_t *buf, size_t buf_size, void *fn,
                               size_t nargs_now, size_t nargs_later,
                               va_list args_now) {
  // Compute how many arguments we have in total
  const size_t nargs_total = nargs_now + nargs_later;
  // Create a pointer we'll use to iterate over the buffer
  uint8_t *cur = buf;

  // It's possible that we're running with CET enabled. Like GCC, we need to
  // emit an `endbr64` as our first instruction.
  {
    // Emit: endbr64
    *cur++ = 0xf3;
    *cur++ = 0x0f;
    *cur++ = 0x1e;
    *cur++ = 0xfa;
  }

  // Allocate space on the stack for overflow args. This also pushes the base
  // pointer onto the stack, which gives us the required alignment. For
  // alignment, we can have an even number of slots in addition to the base
  // pointer.
  {
    const size_t slots_overflow = nargs_total > 6 ? nargs_total - 6 : 0;
    const size_t slots_padded =
        slots_overflow % 2 == 1 ? slots_overflow + 1 : slots_overflow;
    const size_t bytes_padded = 8 * slots_padded;
    assert(bytes_padded % 16 == 0 && "Stack misaligned");
    assert(bytes_padded < UINT32_C(0x10000) && "Too many overflow-args");
    // Emit: enter $(bytes_padded), 0
    // Pushes bytes_padded + 8 bytes onto the stack
    *cur++ = 0xc8;
    *((uint16_t *)cur) = bytes_padded;
    cur += 2;
    *cur++ = 0x00;
  }

  // Place all the later-reg-args in the right place
  const size_t nargs_later_reg = nargs_later > 6 ? 6 : nargs_later;
  for (size_t i = 0; i < nargs_later_reg; i++) {
    // Make sure to iterate in reverse order so we don't accidentally clobber a
    // later argument. Move this argument to a place offset by the number of
    // arguments that we're given now.
    const size_t isrc = nargs_later_reg - i - 1;
    const size_t idst = nargs_now + isrc;
    assert(isrc < 6);

    reg_id_t rid_src = argidx_to_regid(isrc);
    if (idst < 6) {
      // The target is a register
      reg_id_t rid_dst = argidx_to_regid(idst);
      cur = emit_mov_reg_reg(cur, rid_dst, rid_src);
    } else {
      // The target is on the stack
      cur = emit_mov_rsp_reg(cur, idst - 6, rid_src);
    }
  }

  // All of the later-overflow-args need to be copied from the stack
  for (size_t isrc = 6; isrc < nargs_later; isrc++) {
    cur = emit_mov_reg_rbp(cur, REG_ID_RAX, isrc - 6);
    cur = emit_mov_rsp_reg(cur, nargs_now + isrc - 6, REG_ID_RAX);
  }

  // Finally, place all the now-args by materializing them into registers or the
  // stack.
  for (size_t idst = 0; idst < nargs_now; idst++) {
    // Fetch the immediate to materialize
    const uint64_t arg = va_arg(args_now, uint64_t);
    // Materialize it
    if (idst < 6) {
      // The argument goes into a register
      reg_id_t rid_dst = argidx_to_regid(idst);
      cur = emit_mov_reg_imm(cur, rid_dst, arg);
    } else {
      cur = emit_mov_reg_imm(cur, REG_ID_RAX, arg);
      cur = emit_mov_rsp_reg(cur, idst - 6, REG_ID_RAX);
    }
  }

  // Do the call. We don't know where the destination is, and x86-64 doesn't
  // have a way to call arbitrary 64-bit addresses. Thus, we have to materialize
  // the callsite into a register, then emit an indirect call.
  cur = emit_mov_reg_imm(cur, REG_ID_RAX, (uint64_t)fn);
  {
    // Emit: call %rax
    *cur++ = 0xff;
    *cur++ = 0xd0;
  }
  // Did that call. Now all that's left is to get back to the caller.
  {
    // Emit: leave
    *cur++ = 0xc9;
  }

  // Currently, the return value is in %rax. We need to free this buffer though.
  // So, push the return value onto the stack, free ourselves, and have the call
  // return to a trampoline that restores the return value before returning. Of
  // course, the trampoline has to be statically allocated so we don't have to
  // free it. Additionally, the trampoline cannot rely on the stack to be
  // aligned 8 mod 16. In fact, it will be aligned 0 mod 16.
  {
    // Emit: push %rax
    *cur++ = 0x50;
    // Emit: mov %rax, $(vcurry_return_trampoline); push %rax
    cur = emit_mov_reg_imm(cur, REG_ID_RAX, (uint64_t)&vcurry_return_trampoline);
    *cur++ = 0x50;

    // Setup arguments for `munmap`
    cur = emit_mov_reg_imm(cur, REG_ID_RDI, (uint64_t)buf);
    cur = emit_mov_reg_imm(cur, REG_ID_RSI, buf_size);

    // Emit: mov %rax, $(munmap); jmp %rax
    cur = emit_mov_reg_imm(cur, REG_ID_RAX, (uint64_t)munmap);
    *cur++ = 0xff;
    *cur++ = 0xe0;
  }

  // Make sure we didn't overrun the buffer
  assert((size_t)(cur - buf) <= buf_size);
}

static reg_id_t argidx_to_regid(size_t argidx) {
  // Bounds checking
  assert(argidx < 6 && "Invalid index for register argument");
  // Return from a lookup table
  const reg_id_t lut[] = {REG_ID_RDI, REG_ID_RSI, REG_ID_RDX,
                          REG_ID_RCX, REG_ID_R8,  REG_ID_R9};
  return lut[argidx];
}

static uint8_t *emit_mov_reg_reg(uint8_t *cur, reg_id_t dst, reg_id_t src) {
  // Here, the r/m64 is the destination, and the r64 is the source
  *cur++ = 0x48 | ((src >> 3) & 1) << 2 | ((dst >> 3) & 1) << 0;
  *cur++ = 0x89;
  *cur++ = 0xc0 | (src & 7) << 3 | (dst & 7) << 0;
  return cur;
}

static uint8_t *emit_mov_rsp_reg(uint8_t *cur, size_t slot, reg_id_t src) {
  // We have different encodings depending on whether the slot's offset fits
  // within an 8-bit signed integer. We can also optimize if the offset is zero.
  bool zero_encoding = slot == 0;
  bool small_encoding = slot < 128 / 8;
  // The prefix is always the same. We encode the source register. The base
  // register is always %rsp, and we never have an index.
  *cur++ = 0x48 | ((src >> 3) & 1) << 2;
  *cur++ = 0x89;
  if (zero_encoding) {
    *cur++ = 0x04 | (src & 7) << 3;
    *cur++ = 0x24;
  } else if (small_encoding) {
    assert(8 * slot < 128);
    *cur++ = 0x44 | (src & 7) << 3;
    *cur++ = 0x24;
    *cur++ = 8 * slot;
  } else {
    *cur++ = 0x84 | (src & 7) << 3;
    *cur++ = 0x24;
    *((int32_t *)cur) = 8 * slot;
    cur += 4;
  }
  return cur;
}

static uint8_t *emit_mov_reg_rbp(uint8_t *cur, reg_id_t dst, size_t slot) {
  // Compute the byte offset to which we will store. Note that this will never
  // be zero since we skip over the saved base pointer and the return address.
  const size_t offset = 16 + 8 * slot;
  // We have different encodings depending on whether the offset fits within an
  // 8-bit signed integer.
  bool small_encoding = offset < 128;
  // The prefix is always the same. We encode the source register. The base
  // register is always %rbp, and we never have an index.
  *cur++ = 0x48 | ((dst >> 3) & 1) << 2;
  *cur++ = 0x8b;
  if (small_encoding) {
    *cur++ = 0x45 | (dst & 7) << 3;
    *cur++ = offset;
  } else {
    *cur++ = 0x85 | (dst & 7) << 3;
    *((int32_t *)cur) = offset;
    cur += 4;
  }
  return cur;
}

static uint8_t *emit_mov_reg_imm(uint8_t *cur, reg_id_t dst, uint64_t imm) {
  // TODO: Optimize. There are are lot of ways to materialize immediates into
  // registers. Ideally, we'd pick the best one.
  if ((dst & 8) == 0 && is_u32(imm)) {
    // If we're using a 32-bit register (high bit clear) and the immediate is
    // unsigned 32-bit, then we can use a shorter encoding.
    *cur++ = 0xb8 | (dst & 7);
    *((uint32_t *)cur) = imm & UINT32_C(0xffffffff);
    cur += 4;
  } else if (is_i32(imm)) {
    // Otherwise, if the immediate is signed 32-bit
    *cur++ = 0x48 | ((dst >> 3) & 1) << 0;
    *cur++ = 0xc7;
    *cur++ = 0xc0 | (dst & 7);
    *((uint32_t *)cur) = imm & UINT32_C(0xffffffff);
    cur += 4;
  } else {
    // Just use the long encoding
    *cur++ = 0x48 | ((dst >> 3) & 1) << 0;
    *cur++ = 0xb8 | (dst & 7);
    *((uint64_t *)cur) = imm;
    cur += 8;
  }
  return cur;
}

static size_t vcurry_estimate_thunk_size(size_t nargs_now, size_t nargs_later) {
  size_t ret = 0;
  // Entry stuff
  ret += 4; // endbr64
  ret += 4; // enter $(bytes_total), 0
  // Setting up the arguments
  ret += (8 + 7) * nargs_later;
  ret += (10 + 8) * nargs_now;
  // Calling
  ret += 10; // mov %rax, $(fn)
  ret += 2;  // call %rax
  // Exit stuff
  ret += 1;  // leave
  ret += 1;  // push %rax
  ret += 10; // mov %rax, $(vcurry_return_trampoline)
  ret += 1;  // push %rax
  ret += 10; // mov %rdi, $(buf)
  ret += 10; // mov %rsi, $(buf_size)
  ret += 10; // mov %rax, $(munmap)
  ret += 2;  // jmp %rax
  return ret;
}
