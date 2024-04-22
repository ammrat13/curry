#include <stdint.h>

#include "curry.h"
#include "unity.h"

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

// Check that we can chain curries together
void test_args16_4_4_4_4(void) {
  void *curry0 = curry(dut_args16, 4, 12, 0x0, 0x1, 0x2, 0x3);
  TEST_ASSERT_NOT_NULL(curry0);
  void *curry1 = curry(curry0, 4, 8, 0x4, 0x5, 0x6, 0x7);
  TEST_ASSERT_NOT_NULL(curry1);
  void *curry2 = curry(curry1, 4, 4, 0x8, 0x9, 0xa, 0xb);
  TEST_ASSERT_NOT_NULL(curry2);
  void *curry3 = curry(curry2, 4, 0, 0xc, 0xd, 0xe, 0xf);
  TEST_ASSERT_NOT_NULL(curry3);
  TEST_ASSERT_EQUAL_UINT64(0xff, ((uint64_t(*)(void))curry3)());
}

// Check that this works even with overflows
void test_args16_7_7_2(void) {
  void *curry0 = curry(dut_args16, 7, 9, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6);
  TEST_ASSERT_NOT_NULL(curry0);
  void *curry1 = curry(curry0, 7, 2, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd);
  TEST_ASSERT_NOT_NULL(curry1);
  void *curry2 = curry(curry1, 2, 0, 0xe, 0xf);
  TEST_ASSERT_NOT_NULL(curry2);
  TEST_ASSERT_EQUAL_UINT64(0xff, ((uint64_t(*)(void))curry2)());
}
