#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "lib.h"

union float_converter {
    uint32_t intval;
    float floatval;
} __attribute__((packed));

static float_t example_val = {2, 3, 4};

// NOT IN HANDOUT ====================================================
bool is_zero(float_t a) { return a.mantissa == 0 && a.exponent == 0; }

// Get first x bits from given variable (after ignoring initial zeros)
// If there are are not enough bits, then append zeros
uint64_t get_x_bits(size_t bits, uint64_t val) {
    unsigned char *b = (unsigned char *)&val;

    uint64_t answer = 0;
    size_t counter = 0;
    for (int i = sizeof(val) - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            unsigned char byte = b[i] & (1 << j);
            byte >>= j;
            if (counter == 0 && byte == 0) {
                continue;
            } else if (counter == bits) {
                break;
            }

            answer = (answer << 1) | byte;
            counter = counter + 1;
        }
    }

    // append zeros for the bits which are not present in the val.
    while (counter < bits) {
        answer = (answer << 1);
        counter = counter + 1;
    }
    return answer;
}
#define IMPLIED_BIT (1)
float_t fp_round_even(float_t f) {
    float_t answer = f;

    uint8_t G = get_x_bits((FRAC_BITS + IMPLIED_BIT), f.mantissa) % 2;
    uint8_t R = get_x_bits((FRAC_BITS + IMPLIED_BIT + 1), f.mantissa) % 2;
    uint8_t S = 0;

    size_t bl = get_bit_len(f.mantissa);
    if (bl <= (FRAC_BITS + IMPLIED_BIT + 1)) {
        S = 0;
    } else {
        size_t extraBits = bl - (FRAC_BITS + IMPLIED_BIT + 1);
        uint64_t mask = ((UINT64_C(1) << extraBits) - 1);
        if (f.mantissa & mask) {
            S = 1;
        }
    }

    answer.mantissa = get_x_bits((FRAC_BITS + IMPLIED_BIT), f.mantissa);

    // Increment mantissa
    if ((G && R) || (R && S)) {
        answer.mantissa += 1;
    }

    return answer;
}

// Get the size in bits for given variable (after ignoring initial zeros)
size_t get_bit_len(uint64_t val) {
    size_t i = 0;
    uint64_t mask = ~0;
    for (; i < (sizeof(val) * 8); ++i) {
        if ((val & mask) == 0) {
            return i;
        }
        mask = mask << 1;
    }
    return i;
}

// Remove first x  bits from given variable (after ignoring initial zeros)
// If there are are not enough bits, then return 0
uint64_t remove_x_bits(size_t bits, uint64_t val) { return (val >> bits); }

// Convert the float into computation friendly format
float_t remove_fraction_part(float_t f) {
    // Making mantissa non-fraction as I don't want to
    // perform computations on fractional mantissa
    size_t mlen = (FRAC_BITS + 1);  // with 1 bit for implied 1.
    f.exponent -= mlen;             // adding additional exponent for the movement
    return f;
}

// Convert the float into computation friendly format
float_t revert_fraction_part(float_t f) {
    //  Making mantissa non-fraction as I don't want to
    //  perform computations on fractional mantissa
    size_t mlen = get_bit_len(f.mantissa);
    f.exponent += mlen;  // Removing the added exponent
    return f;
}

// Convert the float into computation friendly format
float_t fp_denormalize(float_t f) {
    float_t answer;
    answer.sign = f.sign;
    answer.exponent = (f.exponent - BIAS(EXPONENT_BITS));
    answer.mantissa = f.mantissa;

    if (f.exponent == 0) {
        // Denormalized case
        answer.exponent = 1 - BIAS(EXPONENT_BITS);
        return answer;
    }
    if (f.exponent == 0xff) {
        // Special case
        if (f.mantissa == 0) {
            assert(!"Operand is Infinity\n");
        } else {
            // Not a Number
            assert(!"Operand is Not a valid float Number\n");
        }
        return answer;
    }

    // Normalized float
    answer.mantissa = (UINT64_C(1) << FRAC_BITS) | f.mantissa;
    answer.exponent += 1;  // Removing implied 1

    return answer;
}

// Convert back the float into normalized format so that it is easy to encode
float_t fp_normalize(float_t f) {
    float_t answer = f;

    if ((f.exponent + BIAS(EXPONENT_BITS)) == 0) {
        // Denormalized case
        answer.exponent = 0;
        // Making mantissa 23 bits from 24 bits
        answer.mantissa = answer.mantissa >> 1;
        return answer;
    }
    if (answer.exponent == 0xff) {
        // Special case
        if (f.mantissa == 0) {
            assert(!"Operand is Infinity\n");
        } else {
            // Not a Number
            assert(!"Operand is Not a valid float Number\n");
        }
        return answer;
    }

    // Normalized float
    answer.exponent = (f.exponent + BIAS(EXPONENT_BITS));
    answer.mantissa = answer.mantissa & ((UINT64_C(1) << FRAC_BITS) - 1);
    answer.exponent -= 1;  // removing one bit for implied 1
    return answer;
}
// NOT IN HANDOUT =====================================================

/*
1) Give the bit patterns of each field in the structure when the number
   corresponds to following values:
*/
/*   a) Returns Inf value */
float_t get_Inf(void) {
    float_t answer = example_val;
    // TODO: add your code to represent Inf
    answer.sign = 0;
    answer.exponent = 0xff;
    answer.mantissa = 0;  // Any value execpt zero is fine
    return answer;
}

/*   b) Returns NaN value */
float_t get_NaN(void) {
    float_t answer = example_val;
    // TODO: add your code to represent NaN
    answer.sign = 0;
    answer.exponent = 0xff;
    answer.mantissa = 1;  // Any value execpt zero is fine
    return answer;
}

/*   c) Returns float value for 1 */
float_t get_1(void) {
    float_t answer = example_val;
    // TODO: add your code to represent 1
    answer.sign = 0;
    answer.exponent = (0xff >> 1);
    answer.mantissa = 0;
    return answer;
}

/*   d) Returns float value for 0 */
float_t get_0(void) {
    float_t answer = example_val;
    // TODO: add your code to represent 0
    answer.sign = 0;
    answer.exponent = 0;
    answer.mantissa = 0;
    return answer;
}

/*   e) Returns smallest positive denormalized value */
float_t get_smallest_denormalized(void) {
    float_t answer = example_val;
    // TODO: add your code to represent smallest denormalized value
    answer.sign = 0;
    answer.exponent = 0;
    answer.mantissa = 1;
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

    float_t answer = example_val;
    // TODO: Your implementation here
    union float_converter fc;
    memset(&fc, 0, sizeof(fc));
    fc.floatval = b;

    if (fc.intval >> (EXPONENT_BITS + FRAC_BITS)) {
        answer.sign = 1;
    } else {
        answer.sign = 0;
    }

    answer.exponent = ((fc.intval << 1) >> (FRAC_BITS + 1));

    answer.mantissa = fc.intval & ((UINT64_C(1) << (FRAC_BITS)) - 1);
    return answer;
}

/* b) Given a float_t struct, returns the equivalent float C data type */
float fp_decode(float_t a) {
    // TIP: Just revert what you have done in above function. Easy!!!

    // TODO: Your implementation here
    float answer = 42;
    union float_converter fc;
    memset(&fc, 0, sizeof(fc));

    if (a.sign == 1) {
        fc.intval |= (UINT64_C(1) << (EXPONENT_BITS + FRAC_BITS));
    }

    fc.intval |= ((uint32_t)a.exponent) << FRAC_BITS;
    fc.intval |= ((uint32_t)a.mantissa);
    answer = fc.floatval;
    return answer;
}

/* c) Compute -a, return the result */
float_t fp_negate(float_t a) {
    // TIP: This is be the easiest operation you will be implementing. You just
    // need to flip the sign.

    float_t answer = a;
    // TODO: Your implementation here
    if (answer.sign == 0) {
        answer.sign = 1;
    } else {
        answer.sign = 0;
    }
    return answer;
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

    float_t answer = a;
    // TODO: Your implementation here
    // special cases involving zero
    if (is_zero(a)) return b;
    if (is_zero(b)) return a;

    float_t v1 = remove_fraction_part(fp_denormalize(a));
    float_t v2 = remove_fraction_part(fp_denormalize(b));

    // Assuming that both operator have same sign
    assert(v1.sign == v2.sign);

    float_t ans_raw = v1;

    // Step 1: Making sure that E1 > E2
    if (v1.exponent < v2.exponent) {
        float_t temp = v1;
        v1 = v2;
        v2 = temp;
    }

    // Just as a precaution
    assert(v1.exponent >= v2.exponent);

    // Step 2: Adjust v1 so that it will have same exponent as v2
    int64_t e_gap = v1.exponent - v2.exponent;

    // add e_gap bits into v1 mantissa
    v1.mantissa = v1.mantissa << e_gap;
    v1.exponent -= (e_gap);

    // Just a precaution: Make sure that both have same exponent
    assert(v1.exponent == v2.exponent);

    // Step 3: Now lets do addition
    ans_raw.mantissa = v1.mantissa + v2.mantissa;

    ans_raw.exponent = v1.exponent;

    float_t ans_rounded = fp_round_even(revert_fraction_part(ans_raw));
    answer = fp_normalize(ans_rounded);
    return answer;
}

/* e) Multiply two float_t numbers, return the result */
float_t fp_mul(float_t a, float_t b) {
    // TIP:  Just like in the ``Addition`` operation, mantissa here is the
    // fraction.  You can't use floating point operations to multiply mentisa's.
    // Don't worry about all the corner cases in the beginning, you can extend
    // your solution to incorporate all corner cases later.

    float_t answer = a;
    // TODO: Your implementation here
    // special cases involving zero
    if (is_zero(a)) {
        // return properly signed zero
        b.sign ^= a.sign;
        b.mantissa = b.exponent = 0;
        return b;
    }
    if (is_zero(b)) {
        a.sign ^= b.sign;
        a.mantissa = a.exponent = 0;
        return a;
    }

    float_t v1 = remove_fraction_part(fp_denormalize(a));
    float_t v2 = remove_fraction_part(fp_denormalize(b));

    float_t ans_raw;
    ans_raw.sign = v1.sign ^ v2.sign;
    ans_raw.mantissa = (v1.mantissa * v2.mantissa);
    if (ans_raw.mantissa != 0) {
        ans_raw.exponent = v1.exponent + v2.exponent;
    } else {
        ans_raw.exponent = v1.exponent;
    }

    float_t ans_rounded = fp_round_even(revert_fraction_part(ans_raw));
    answer = fp_normalize(ans_rounded);
    return answer;
}