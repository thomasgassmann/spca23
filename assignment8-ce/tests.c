#include <stdio.h>
#include <assert.h>
#include "lib.h"

void test_fp_add(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_add(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 + f2;

  assert(c_ans == m_ans);
}

void test_fp_mul(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_mul(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 * f2;

  assert(c_ans == m_ans);
}

unsigned int public_tests() {
  unsigned int oks = 0;
  test_fp_add(-1, 1);
  test_fp_mul(1.17549435e-038, 0.000000000001);
  test_fp_mul(3.402823E+38, 3.402823E+38);

  test_fp_add(4.2e-12, 4.2e12);
  test_fp_add(1, 1);
  test_fp_add(323.2, 4.2e12);
  test_fp_add(4.2, 4.3);
  test_fp_add(0.0, 0.0);
  test_fp_add(0, 1);

  test_fp_mul(0.0000001, 0.000000000001);
  test_fp_mul(1, 1);
  test_fp_mul(1, 0);
  test_fp_mul(0, 1);

  float a = 1.17549435e-038;
  float b = 1.17549435e-038;

  return oks;
}

int main() {
  printf("Running tests...\n");
  unsigned int oks = public_tests();
  printf("%d\n", oks);
  return 0;
}
