#include "lib.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

union float_converter {
  uint32_t intval;
  float floatval;
} __attribute__((packed));

typedef struct internal_t {
  uint8_t sign;
  uint8_t is_inf;
  uint8_t is_nan;
  int32_t exponent; // the actual, effective exponent
  uint64_t mantissa; // value to be multiplied with exponent
} internal_t;

#define B BIAS(EXPONENT_BITS)
#define LARGEST_NORMALIZED_EXPONENT 0xFE
#define MAX_EXP 0xFF
#define IS_MAX_EXP(exp) (exp == MAX_EXP)
#define MANTISSA_MASK ((1 << FRAC_BITS) - 1)

void shift_right_mantissa(internal_t *value, int amount);
float_t to_ieee754(internal_t value);

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

void postnormalize(internal_t *value) {
  if ((value->mantissa & MANTISSA_MASK) == 0) {
    // we have an overflow
    shift_right_mantissa(value, 1);
  }
}

// shifts right and rounds as necessary
void shift_right_mantissa(internal_t *value, int amount) {
  value->exponent += amount;
  if (amount >= 64) {
    value->mantissa = 0;
    return;
  }

  uint64_t current_mantissa = value->mantissa;
  uint64_t guard_bit = (current_mantissa >> amount) & 1;  
  uint64_t round_bit = (current_mantissa >> (amount - 1)) & 1;
  uint64_t sticky_mask = ((uint64_t)(1) << (amount - 1)) - 1;
  uint64_t sticky_bits = current_mantissa & sticky_mask;
  uint64_t sticky_bit = !!sticky_bits;
  if ((guard_bit && round_bit && !sticky_bit) || (round_bit && sticky_bit)) {
    // round to even (first case), round up (second case)
    value->mantissa >>= amount;
    // this might overflow again, need to postnormalize
    // if we're really unlucky, this can make make a normalized
    // number out of a denormalized number
    value->mantissa++;
    postnormalize(value);
  } else {
    // we can just shift things out, implicitly round down
    value->mantissa >>= amount;
  }
}

// gets the best denormalized representation
// returns zero if not possible
float_t try_denormalize(internal_t *value) {
  if (value->exponent > 1 - B - FRAC_BITS) {
    // here we need to shift left instead, because
    // we creating a normalized number would result
    // in an exponent lower than the lowest exponent
    uint32_t shift_amount = 0;
    int32_t exp = value->exponent;
    while (exp > 1 - B - FRAC_BITS) {
      shift_amount++;
      exp--;
    }

    float_t res;
    res.exponent = value->exponent - shift_amount + FRAC_BITS + B - 1;
    res.mantissa = value->mantissa << shift_amount;
    res.sign = value->sign;
    return res;
  }

  uint32_t shift_amount = 0;
  int32_t exp = value->exponent;
  while (exp < 1 - B - FRAC_BITS) {
    shift_amount++;
    exp++;
  }

  shift_right_mantissa(value, shift_amount);
  if (value->mantissa == 0) {
    float_t zero = get_0();
    zero.sign = value->sign;
    return zero;
  }

  float_t res;
  res.exponent = value->exponent + FRAC_BITS + B - 1;
  if (res.exponent != 0) {
    // the number was postnormalized and is now normalized again
    // due to an overflow, it should have a leading 1
    return to_ieee754(*value);
  } else {
    res.mantissa = value->mantissa;
  }

  res.sign = value->sign;
  return res;
}

float_t to_ieee754(internal_t value) {
  // remove leading one, if normalized
  if (value.is_nan) {
    return get_NaN();
  }
  
  float_t inf = get_Inf();
  inf.sign = value.sign;
  if (value.is_inf) {
    return inf;
  }

  float_t zero = get_0();
  zero.sign = value.sign;
  if (value.mantissa == 0) {
    return zero;
  }

  float_t res = {value.sign, 0, 0};
  uint64_t rest = value.mantissa >> FRAC_BITS;
  uint64_t mantissa_cutoff = value.mantissa & MANTISSA_MASK;
  if (rest > 1) {
    // shift right
    int amount = 0;
    while (rest > 1) {
      amount++;
      rest >>= 1;
    }

    if (MAX_EXP <= value.exponent + amount) {
      return inf;
    }

    if (value.exponent + B + FRAC_BITS + amount <= 0) {
      float_t close_zero = try_denormalize(&value);

      return close_zero;
    }

    shift_right_mantissa(&value, amount);
    if (value.exponent + B + FRAC_BITS > LARGEST_NORMALIZED_EXPONENT) {
      return inf;
    }

    res.exponent = value.exponent + B + FRAC_BITS;
    res.mantissa = value.mantissa & MANTISSA_MASK;
  } else if (rest == 0) {
    // shift upwards, number might get denormalized
    int amount = 0;
    uint64_t mantissa = value.mantissa;
    while (rest == 0) {
      amount++;
      mantissa <<= 1;
      rest = mantissa >> FRAC_BITS;
    }

    if (value.exponent - amount < 1 - B - FRAC_BITS) {
      // this number is going to be denormalized
      return try_denormalize(&value);
    }

    value.mantissa <<= amount;
    value.exponent -= amount;
    if ((value.mantissa >> FRAC_BITS) == 0) {
      // number is denormalized
      res.exponent = value.exponent + B - 1 + FRAC_BITS;
      res.mantissa = value.mantissa & MANTISSA_MASK;
    } else {
      // number is normalized
      res.exponent = value.exponent + B + FRAC_BITS;
      res.mantissa = value.mantissa & MANTISSA_MASK;
    }
  } else {
    // check if exp is too small
    if (value.exponent < 1 - B - FRAC_BITS) {
      // this number is going to be denormalized
      return try_denormalize(&value);
    }

    // we already have a leading 1
    res.exponent = value.exponent + B + FRAC_BITS;
    res.mantissa = mantissa_cutoff;
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
    res.exponent = value.exponent - B - FRAC_BITS;
  } else {
    // must be denormalized, i.e. value.exponent == 0
    assert(value.exponent == 0);
    res.mantissa = value.mantissa;
    res.exponent = - B + 1 - FRAC_BITS;
  }
  
  return res;
}

void shift_left_to_mantissa_position(internal_t *value) {
  while ((value->mantissa & (1 << FRAC_BITS)) == 0) {
    value->mantissa <<= 1;
    value->exponent--;
  }
}

internal_t add(internal_t a, internal_t b) {
  if ((a.is_inf && b.is_inf && a.sign != b.sign) || a.is_nan || b.is_nan) {
    internal_t not_a_number = {0, 0, 1, 0, 0};
    return not_a_number;
  }
  
  if (a.is_inf || b.is_inf) {
    return a.is_inf ? a : b;
  }

  if (a.mantissa == 0 || b.mantissa == 0) {
    return a.mantissa == 0 ? b : a;
  }

  shift_left_to_mantissa_position(&a);
  shift_left_to_mantissa_position(&b);
  if (b.exponent > a.exponent) {
    internal_t tmp = a;
    a = b;
    b = tmp;
  }

  // we assume a and b are "normalized" in the sense that not more than
  // the 24 least significant bits are set in the mantissa (i.e. 23 frac bits
  // and 1 implied bit that we might have added).

  // using b.exponent as the new exponent is a bad idea because we might need
  // to shift things outside

  // a.exponent >= b.exponent
  int diff = a.exponent - b.exponent;
  
  internal_t result = {a.sign, 0, 0, b.exponent, 0};
  if (diff > 64 - FRAC_BITS - 1) {
    // b is too insignificant, the result is going to be a
    // (works because of the assumption above)
    result.exponent = a.exponent;
    result.mantissa = a.mantissa;
    return result;
  }

  uint64_t shifted = b.mantissa;
  uint64_t first = a.mantissa << diff;

  if (a.sign != b.sign) {
    if (shifted > first) {
      result.mantissa = shifted - first;
    } else {
      result.mantissa = first - shifted;
    }

    result.sign = a.sign == 1 ? (first > shifted) : (first < shifted);
  } else {
    result.mantissa = first + shifted;
  }
  
  return result;
}

/* d) Add two float_t numbers, return the result */
float_t fp_add(float_t a, float_t b) {
  internal_t pa = from_ieee754(a);
  internal_t pb = from_ieee754(b);
  
  internal_t res = add(pa, pb);
  
  return to_ieee754(res);
}

internal_t mult(internal_t a, internal_t b) {
  if (a.is_nan || b.is_nan) {
    return a.is_nan ? a : b;
  }

  if ((a.is_inf && b.mantissa == 0) ||(b.is_inf && a.mantissa == 0)) {
    a.is_nan = 1;
    return a;
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
  internal_t pa = from_ieee754(a);
  internal_t pb = from_ieee754(b);
  
  internal_t res = mult(pa, pb);
  
  return to_ieee754(res);
}
