#include <stdint.h>

#include "curry.h"
#include "unity.h"

uint64_t dut_args2(uint64_t a0, uint64_t a1) {
  TEST_ASSERT_EQUAL_UINT64(a0, 0x0);
  TEST_ASSERT_EQUAL_UINT64(a1, 0x1);
  return UINT64_C(0xff);
}

// Check that currying one argument at a time to a two-argument function works
// correctly
void test_args2_1_1(void) {
  uint64_t (*const curried)(uint64_t) = curry(dut_args2, 1, 1, 0x0);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x1));
}

// Check that curried arguments are passed correctly no matter how big they are.
// First test is for things that can fit in u32.
uint64_t dut_args1_u32(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0xffff1234));
  return UINT64_C(0xff);
}
void test_args1_u32(void) {
  uint64_t (*const curried)(void) = curry(dut_args1_u32, 1, 0, UINT64_C(0xffff1234));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// Same as above for things that can fit in i32
uint64_t dut_args1_i32(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0xffffffff80000000));
  return UINT64_C(0xff);
}
void test_args1_i32(void) {
  uint64_t (*const curried)(void) = curry(dut_args1_i32, 1, 0, UINT64_C(0xffffffff80000000));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}

// Same as above for anything else
uint64_t dut_args1_general(uint64_t a0) {
  TEST_ASSERT_EQUAL_UINT64(a0, UINT64_C(0x0123456789abcdef));
  return UINT64_C(0xff);
}
void test_args1_general(void) {
  uint64_t (*const curried)(void) = curry(dut_args1_general, 1, 0, UINT64_C(0x0123456789abcdef));
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried());
}
