#include <stdint.h>

#include "curry.h"
#include "unity.h"

static uint64_t dut_args8(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                          uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
  TEST_ASSERT_EQUAL_UINT64(a0, 0x0);
  TEST_ASSERT_EQUAL_UINT64(a1, 0x1);
  TEST_ASSERT_EQUAL_UINT64(a2, 0x2);
  TEST_ASSERT_EQUAL_UINT64(a3, 0x3);
  TEST_ASSERT_EQUAL_UINT64(a4, 0x4);
  TEST_ASSERT_EQUAL_UINT64(a5, 0x5);
  TEST_ASSERT_EQUAL_UINT64(a6, 0x6);
  TEST_ASSERT_EQUAL_UINT64(a7, 0x7);
  return 0xff;
}

static uint64_t dut_args16(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                           uint64_t a8, uint64_t a9, uint64_t aa, uint64_t ab,
                           uint64_t ac, uint64_t ad, uint64_t ae, uint64_t af) {
  TEST_ASSERT_EQUAL_UINT64(a0, 0x0);
  TEST_ASSERT_EQUAL_UINT64(a1, 0x1);
  TEST_ASSERT_EQUAL_UINT64(a2, 0x2);
  TEST_ASSERT_EQUAL_UINT64(a3, 0x3);
  TEST_ASSERT_EQUAL_UINT64(a4, 0x4);
  TEST_ASSERT_EQUAL_UINT64(a5, 0x5);
  TEST_ASSERT_EQUAL_UINT64(a6, 0x6);
  TEST_ASSERT_EQUAL_UINT64(a7, 0x7);
  TEST_ASSERT_EQUAL_UINT64(a8, 0x8);
  TEST_ASSERT_EQUAL_UINT64(a9, 0x9);
  TEST_ASSERT_EQUAL_UINT64(aa, 0xa);
  TEST_ASSERT_EQUAL_UINT64(ab, 0xb);
  TEST_ASSERT_EQUAL_UINT64(ac, 0xc);
  TEST_ASSERT_EQUAL_UINT64(ad, 0xd);
  TEST_ASSERT_EQUAL_UINT64(ae, 0xe);
  TEST_ASSERT_EQUAL_UINT64(af, 0xf);
  return 0xff;
}

// Check that we push excess arguments onto the stack correctly, even if the
// original calls don't use the stack.
void test_args8_4_4(void) {
  uint64_t (*const curried)(uint64_t, uint64_t, uint64_t, uint64_t) =
      curry(dut_args8, 4, 4, 0x0, 0x1, 0x2, 0x3);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x4, 0x5, 0x6, 0x7));
}

// Check that now-overflow-args are handled correctly
void test_args8_7_1(void) {
  uint64_t (*const curried)(uint64_t) =
      curry(dut_args8, 7, 1, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x7));
}
// Check that later-overflow-args are handled correctly
void test_args8_1_7(void) {
  uint64_t (*const curried)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                            uint64_t, uint64_t) = curry(dut_args8, 1, 7, 0x0);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff, curried(0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7));
}

// Check that currying works even when both now and later overflow
void test_args16_8_8(void) {
  uint64_t (*const curried)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                            uint64_t, uint64_t, uint64_t) =
      curry(dut_args16, 8, 8, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7);
  TEST_ASSERT_NOT_NULL(curried);
  TEST_ASSERT_EQUAL_UINT64(0xff,
                           curried(0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf));
}
