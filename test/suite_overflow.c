#include <stdint.h>

#include "curry.h"
#include "unity.h"

uint64_t dut_args8(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                   uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
  TEST_ASSERT_EQUAL_UINT64(a0, 0x0);
  TEST_ASSERT_EQUAL_UINT64(a1, 0x1);
  TEST_ASSERT_EQUAL_UINT64(a2, 0x2);
  TEST_ASSERT_EQUAL_UINT64(a3, 0x3);
  TEST_ASSERT_EQUAL_UINT64(a4, 0x4);
  TEST_ASSERT_EQUAL_UINT64(a5, 0x5);
  TEST_ASSERT_EQUAL_UINT64(a6, 0x6);
  TEST_ASSERT_EQUAL_UINT64(a7, 0x7);
  return UINT64_C(0xff);
}

// Check that we push excess arguments onto the stack correctly, even if the
// original calls don't use the stack.
void test_args8_4_4(void) {
  uint64_t (*const curried)(uint64_t, uint64_t, uint64_t, uint64_t) =
      curry(dut_args8, 4, 4, 0x0, 0x1, 0x2, 0x3);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x4, 0x5, 0x6, 0x7));
}
