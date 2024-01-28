#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include "lib.h"

int test_fp_add(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_add(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 + f2;

  return c_ans == m_ans;
}

int test_fp_mul(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_mul(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 * f2;

  return c_ans == m_ans;
}

void public_tests() {
  test_fp_add(-1, 1);
  test_fp_mul(1.17549435e-038, 0.000000000001);
  test_fp_mul(3.402823E+38, 3.402823E+38);

  test_fp_add(4.2e-12, 4.2e12);
  test_fp_add(1, 1);
  test_fp_add(323.2, 4.2e12);
  test_fp_add(4.2, 4.3);
  test_fp_add(0.0, 0.0);
  test_fp_add(0, 1);
  test_fp_add(0, -1);
  test_fp_add(-1, -1);

  test_fp_mul(0.0000001, 0.000000000001);
  test_fp_mul(1, 1);
  test_fp_mul(1, 0);
  test_fp_mul(0, 1);
  test_fp_mul(-1, 1);
  test_fp_mul(-1, 50);
  test_fp_mul(-1, 4.2e13);
  test_fp_mul(-4.2e-13, 4.2e13);
  test_fp_mul(-4.2e-1, 4.2e13);

  union {
    float f;
    uint32_t i;
  } value_a = {0};
  union {
    float f;
    uint32_t i;
  } value_b = {0};
  uint64_t oks = 0;
  for (uint32_t v = 0; 1; v++) {
    for (uint32_t h = 0; 1; h++) {
      value_a.i = v;
      value_b.i = h;

      int current = 0;
      current += test_fp_add(value_a.f, value_b.f);
      current += test_fp_mul(value_a.f, value_b.f);
      if (current != 2) {
        printf("%f - %f failed\n", value_a.f, value_b.f);
      }

      oks += current;

      if ((oks & 0xFFFFF) == 0) {
        printf("%" PRId64 "\r", oks);
      }

      if (h == 0xFFFFFFFF) {
        break;
      }
    }

    if (v == 0xFFFFFFFF) {
      break;
    }
  }
}

int main() {
  printf("Running tests...\n");
  public_tests();
  printf("Done!\n");
  return 0;
}
