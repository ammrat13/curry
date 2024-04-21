#include <stdint.h>

#include "curry.h"
#include "unity.h"

// Check that currying one argument at a time to a two-argument function works
// correctly
uint64_t dut_args2(uint64_t a, uint64_t b) {
  TEST_ASSERT_EQUAL_UINT64(a, 0x0);
  TEST_ASSERT_EQUAL_UINT64(b, 0x1);
  return UINT64_C(0xff);
}
void test_args2(void) {
  uint64_t (*const curried)(uint64_t) = curry(dut_args2, 1, 1, 0x0);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x1));
}
