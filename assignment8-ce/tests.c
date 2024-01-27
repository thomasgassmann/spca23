#include <stdio.h>
#include "lib.h"

int test_fp_add(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_add(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 + f2;

  return c_ans == m_ans ? 1 : 0;
}

int test_fp_mul(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_mul(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 * f2;

  return c_ans == m_ans ? 1 : 0;
}

unsigned int public_tests() {
  unsigned int oks = 0;
  oks += test_fp_add(4.2e-12, 4.2e12);
  oks += test_fp_add(1, 1);
  oks += test_fp_add(-1, 1);
  oks += test_fp_add(323.2, 4.2e12);
  oks += test_fp_add(4.2, 4.3);
  oks += test_fp_add(0.0, 0.0);
  oks += test_fp_add(0, 1);

  oks += test_fp_mul(1, 1);
  oks += test_fp_mul(1, 0);
  oks += test_fp_mul(0, 1);
  oks += test_fp_mul(3.402823E+38, 3.402823E+38);
  return oks;
}

int main() {
  printf("Running tests...\n");
  unsigned int oks = public_tests();
  printf("%d\n", oks);
  return 0;
}
