#include "lib.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

union float_converter {
  uint32_t intval;
  float floatval;
} __attribute__((packed));

typedef struct internal_t {
  uint8_t sign;
  uint8_t is_inf;
  uint8_t is_nan;
  int32_t exponent;
  uint64_t mantissa;
} internal_t;

#define B BIAS(EXPONENT_BITS)
#define LARGEST_NORMALIZED_EXPONENT 0xFE
#define IS_MAX_EXP(exp) (exp == 0xFF)

bool is_zero(float_t f) {
  return f.mantissa == 0 && f.exponent == 0;
} 

bool is_normalized(float_t f) {
  return f.exponent > 0 && f.exponent <= LARGEST_NORMALIZED_EXPONENT;
}

bool is_NaN(float_t f) {
  return IS_MAX_EXP(f.exponent) && f.mantissa != 0;
}

bool is_inf(float_t f) {
  return IS_MAX_EXP(f.exponent) && f.mantissa == 0;
}

bool is_posinf(float_t f) {
  return is_inf(f) && f.sign == 0;
}

bool is_neginf(float_t f) {
  return is_inf(f) && f.sign == 1;
}

/*
1) Give the bit patterns of each field in the structure when the number
   corresponds to following values:
*/
/*   a) Returns Inf value */
float_t get_Inf(void) {
  float_t answer = {0, 0xFF, 0};
  return answer;
}

/*   b) Returns NaN value */
float_t get_NaN(void) {
  float_t answer = {0, 0xFF, 1};
  return answer;
}

/*   c) Returns float value for 1 */
float_t get_1(void) {
  float_t answer = {0, B, 0};
  return answer;
}

/*   d) Returns float value for 0 */
float_t get_0(void) {
  float_t answer = {0, 0, 0};
  return answer;
}

/*   e) Returns smallest positive denormalized value */
float_t get_smallest_denormalized(void) {
  float_t answer = {0, 0, 1};
  return answer;
}

/*
2) Implement the body of the prototypes:
*/

/* a) Given a C float data type, returns the equivalent float_t data
 * struct */
float_t fp_encode(float b) {
  // TIP: You may want to use a union with a uint32_t, shift and bitwise
  // operators for this.  Also, think about whether the endianness of your
  // machine will affect the way you want to interpret the float.
  union float_converter converter;
  converter.floatval = b;
  int bits = converter.intval;
  
  float_t result = {0, 0, 0};
  result.sign = !!(bits & 0x80000000);
  result.exponent = (bits & 0x7F800000) >> FRAC_BITS;
  result.mantissa = bits & 0x007FFFFF;
  
  return result;
}

/* b) Given a float_t struct, returns the equivalent float C data type */
float fp_decode(float_t a) {
  // TIP: Just revert what you have done in above function. Easy!!!
  union float_converter converter = {0};
  converter.intval = a.mantissa;
  converter.intval |= a.sign << (FRAC_BITS + EXPONENT_BITS);
  converter.intval |= (a.exponent << FRAC_BITS);
  
  return converter.floatval;
}

/* c) Compute -a, return the result */
float_t fp_negate(float_t a) {
  // TIP: This is be the easiest operation you will be implementing. You just
  // need to flip the sign.
  a.sign = !a.sign;
  return a;
}

void shift_right_mantissa(internal_t *value) {
  value->exponent++;
  value->mantissa >>= 1;
  
  // check overflow
  if (IS_MAX_EXP(value->exponent)) {
    value->is_inf = 1;
  }
}

float_t to_ieee754(internal_t value) {
  // remove leading one, if normalized
  if (value.is_nan) {
    return get_NaN();
  }
  
  if (value.is_inf) {
    float_t inf = get_Inf();
    inf.sign = value.sign;
    return inf;
  }

  float_t res = {value.sign, 0, 0};
  uint64_t rest = value.mantissa >> FRAC_BITS;
  res.mantissa = value.mantissa & ((1 << FRAC_BITS) - 1);
  res.exponent = value.exponent;
  if (rest > 1) {
    while (rest > 1) {
      shift_right_mantissa(&res);
      rest >>= 1;
    }

    res.exponent += B;
  } else if (rest == 0) {
    // shift upwards, number might get denormalized
    res.exponent += B - 1;
  } else {
    // we already have a leading 1
    res.exponent += B;
  }

  
  return res;
}

internal_t from_ieee754(float_t value) {
  // add leading one, if normalized
  internal_t res = {value.sign, 0, 0, 0, 0};
  if (is_inf(value)) {
    res.is_inf = true;
    return res;
  }
  
  if (is_NaN(value)) {
    res.is_nan = true;
    return res;
  }
  
  if (is_normalized(value)) {
    res.mantissa = value.mantissa | (1 << FRAC_BITS);
    res.exponent = value.exponent - B;
  } else {
    // must be denormalized, i.e. value.exponent == 0
    assert(value.exponent == 0);
    res.mantissa = value.mantissa;
    res.exponent = - B + 1;
  }
  
  return res;
}

internal_t add(internal_t a, internal_t b) {
  if ((a.is_inf && b.is_inf && a.sign != b.sign) || a.is_nan || b.is_nan) {
    internal_t not_a_number = {0, 0, 1, 0, 0};
    return not_a_number;
  }
  
  if (a.is_inf || b.is_inf) {
    return a.is_inf ? a : b;
  }
  
  if (b.exponent > a.exponent) {
    internal_t tmp = a;
    a = b;
    b = tmp;
  }
  
  // a.exponent >= b.exponent
  int diff = a.exponent - b.exponent;
  internal_t result = {0, 0, 0, a.exponent, 0};
  
  int64_t shifted = b.mantissa >> diff;
  int64_t first = a.mantissa;
  
  result.mantissa = first + shifted;
  
  return result;
}

/* d) Add two float_t numbers, return the result */
float_t fp_add(float_t a, float_t b) {
  // TIP: Refer to the lecture slides about floating point numbers to implement
  // this operation.  You may also want to implement functions for
  // denormalization, normalization, round_even to implement this operation.
  //
  // Remember that the mantissa in floating point numbers is a fraction and
  // hence leading and trailing zeroes have different semantics than for normal
  // numbers.  Also note that you can't use floating point addition to add
  // mantissas.
  internal_t pa = from_ieee754(a);
  internal_t pb = from_ieee754(b);
  
  internal_t res = add(pa, pb);
  
  return to_ieee754(res);
}

internal_t mult(internal_t a, internal_t b) {
  if (a.is_nan || b.is_nan) {
    return a.is_nan ? a : b;
  }
  
  if (a.is_inf || b.is_inf) {
    a.is_inf = true;
    a.sign ^= b.sign;
    return a;
  }
  
  internal_t res = {
    a.sign ^ b.sign,
    0,
    0,
    a.exponent + b.exponent,
    a.mantissa * b.mantissa
  };
  
  return res;
}

/* e) Multiply two float_t numbers, return the result */
float_t fp_mul(float_t a, float_t b) {
  // TIP:  Just like in the ``Addition`` operation, mantissa here is the
  // fraction.  You can't use floating point operations to multiply mentisa's.
  // Don't worry about all the corner cases in the beginning, you can extend
  // your solution to incorporate all corner cases later.
  internal_t pa = from_ieee754(a);
  internal_t pb = from_ieee754(b);
  
  internal_t res = mult(pa, pb);
  
  return to_ieee754(res);
}
