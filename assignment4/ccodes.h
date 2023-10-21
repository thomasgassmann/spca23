#include <stdint.h>

typedef uint64_t reg_flags_t;

struct ccodes {
  char cf, zf, sf, of; // carry, zero, sign, overflow
 };
struct ccodes getccodes(reg_flags_t eflags);
