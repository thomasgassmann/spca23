#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "lib.h"

float float_from_int(uint32_t value) {
  union {
    float f;
    uint32_t i;
  } c = {0};
  c.i = value;
  return c.f;
}

int isNaN(float value) {
  union {
    float f;
    uint32_t i;
  } c = {value};

  int32_t rest = c.i >> 23;
  int32_t lower = c.i & 0x7FFFFF;
  return lower != 0 && (rest == 0xFF || rest == 0x1FF);
}

int test_fp_add(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_add(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 + f2;
  if (isNaN(m_ans) && isNaN(c_ans)) {
    return 1;
  }

  return c_ans == m_ans;
}

int test_fp_mul(float f1, float f2) {
  float_t v1 = fp_encode(f1);
  float_t v2 = fp_encode(f2);

  float_t ans = fp_mul(v1, v2);
  float m_ans = fp_decode(ans);

  float c_ans = f1 * f2;
  if (isNaN(m_ans) && isNaN(c_ans)) {
    return 1;
  }

  return c_ans == m_ans;
}

int32_t get_random() {
  int x = rand() & 0xff;
  x |= (rand() & 0xff) << 8;
  x |= (rand() & 0xff) << 16;
  x |= (rand() & 0xff) << 24;

  return x;
}

void public_tests() {
  assert(test_fp_add(float_from_int(0x5afaa70d), float_from_int(0xdbb5b0d8)));
  assert(test_fp_mul(float_from_int(0x8ef35bac), float_from_int(0x9b2dd74c)));
  assert(test_fp_mul(float_from_int(0x83bc62af), float_from_int(0x3a2f69b4)));
  assert(test_fp_add(-1, 1));
  assert(test_fp_add(float_from_int(0x50e3cd33), float_from_int(0x5c154748)));
  assert(test_fp_add(float_from_int(0xd0cb0bca), float_from_int(0xbd644748)));
  assert(test_fp_add(float_from_int(0xabbacd29), float_from_int(0x46e3fbf2)));
  assert(test_fp_mul(0, 0.0));
  assert(test_fp_add(0, 0.0));
  assert(test_fp_mul(0, 1.0 / 0.0));
  assert(test_fp_add(0, 1.0 / 0.0));
  assert(test_fp_add(0, -1.0 / 0.0));
  assert(test_fp_mul(0, -1.0 / 0.0));
  assert(test_fp_add(0, 0.0 / 0.0));
  assert(test_fp_mul(0, 0.0 / 0.0));
  assert(test_fp_mul(1.17549435e-038, 0.000000000001));
  assert(test_fp_mul(3.402823E+38, 3.402823E+38));

  assert(test_fp_add(4.2e-12, 4.2e12));
  assert(test_fp_add(1, 1));
  assert(test_fp_add(323.2, 4.2e12));
  assert(test_fp_add(4.2, 4.3));
  assert(test_fp_add(0.0, 0.0));
  assert(test_fp_add(0, 1));
  assert(test_fp_add(0, -1));
  assert(test_fp_add(-1, -1));

  assert(test_fp_mul(0.0000001, 0.000000000001));
  assert(test_fp_mul(1, 1));
  assert(test_fp_mul(1, 0));
  assert(test_fp_mul(0, 1));
  assert(test_fp_mul(-1, 1));
  assert(test_fp_mul(-1, 50));
  assert(test_fp_mul(-1, 4.2e13));
  assert(test_fp_mul(-4.2e-13, 4.2e13));
  assert(test_fp_mul(-4.2e-1, 4.2e13));

  while (1) {
    int32_t value_a = get_random();
    int32_t value_b = get_random();
    printf("0x%x - 0x%x in progress\n", value_a, value_b);
    float a = float_from_int(value_a);
    float b = float_from_int(value_b);
    if (!test_fp_add(a, b)) {
      printf("0x%x - 0x%x add failed\n", value_a, value_b);
      break;
    }

    if (!test_fp_mul(a, b)) {
      printf("0x%x - 0x%x mul failed\n", value_a, value_b);
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
