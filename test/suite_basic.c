#include <stdint.h>

#include "curry.h"
#include "unity.h"

// -----------------------------------------------------------------------------

static uint64_t dut_args2(uint64_t a0, uint64_t a1) {
  TEST_ASSERT_EQUAL_UINT64(a0, 0x0);
  TEST_ASSERT_EQUAL_UINT64(a1, 0x1);
  return 0xff;
}

// Check that currying one argument at a time to a two-argument function works
// correctly
void test_args2_1_1(void) {
  uint64_t (*const curried)(uint64_t) = curry(dut_args2, 1, 1, 0x0);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x1));
}

// Similarly, test currying no arguments now ...
void test_args2_0_2(void) {
  uint64_t (*const curried)(uint64_t, uint64_t) = curry(dut_args2, 0, 2);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x0, 0x1));
}
// ... and both arguments now
void test_args2_2_0(void) {
  uint64_t (*const curried)(void) = curry(dut_args2, 2, 0, 0x0, 0x1);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// -----------------------------------------------------------------------------
// Check the alignment of the stack when the function is called. It should
// always be 8 mod 16, regardless of how many arguments are passed.

// This function returns the stack pointer. It's the caller's job to check that
// it's aligned correctly.
__attribute__((naked)) static uint64_t dut_alignment(void) {
  asm volatile("movq %%rsp, %%rax\n\t"
               "retq"
               :
               :
               : "rax");
}

void test_alignment_1(void) {
  uint64_t (*const curried)(void) = curry(dut_alignment, 1, 0, 0xaa);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0x8, curried() & 0xf);
}

void test_alignment_7(void) {
  uint64_t (*const curried)(void) =
      curry(dut_alignment, 7, 0, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0x8, curried() & 0xf);
}

void test_alignment_8(void) {
  uint64_t (*const curried)(void) = curry(dut_alignment, 8, 0, 0xaa, 0xaa, 0xaa,
                                          0xaa, 0xaa, 0xaa, 0xaa, 0xaa);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0x8, curried() & 0xf);
}
