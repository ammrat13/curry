#include <stdint.h>

#include "curry.h"
#include "unity.h"

// -----------------------------------------------------------------------------
// Check that arguments in registers are passed correctly no matter how big they
// are.

// For things that can fit in u32
static uint64_t dut_reg_u32(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0xffff1234));
  return 0xff;
}
void test_reg_u32(void) {
  uint64_t (*const curried)(void) =
      curry(dut_reg_u32, 1, 0, UINT64_C(0xffff1234));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// For things that can fit in i32
static uint64_t dut_reg_i32(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0xffffffff80000000));
  return 0xff;
}
void test_reg_i32(void) {
  uint64_t (*const curried)(void) =
      curry(dut_reg_i32, 1, 0, UINT64_C(0xffffffff80000000));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// For general values
static uint64_t dut_reg_general(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0x0123456789abcdef));
  return 0xff;
}
void test_reg_general(void) {
  uint64_t (*const curried)(void) =
      curry(dut_reg_general, 1, 0, UINT64_C(0x0123456789abcdef));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// -----------------------------------------------------------------------------
// Check that currying works even with an obscene number of arguments

static uint64_t dut_argsvar(uint64_t nargs, ...) {
  TEST_ASSERT_EQUAL(nargs, 64);
  va_list args;
  va_start(args, nargs);
  for (uint64_t i = 0; i < nargs; i++) {
    TEST_ASSERT_EQUAL_UINT64(va_arg(args, uint64_t), 0xaa);
  }
  va_end(args);
  return 0xff;
}
void test_argsmany(void) {
  void *curried0 = curry(dut_argsvar, 1, 64, 64);
  TEST_ASSERT_NOT_NULL(curried0);
  void *curried1 = curry(curried0, 32, 32, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa);
  TEST_ASSERT_NOT_NULL(curried1);
  void *curried2 = curry(curried1, 32, 0, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa);
  TEST_ASSERT_NOT_NULL(curried2);
  TEST_ASSERT_EQUAL_UINT64(0xff, ((uint64_t(*)(void))curried2)());
}
