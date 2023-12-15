#ifndef CYCLECLOCK_H_
#define CYCLECLOCK_H_

#include <stdint.h>

static inline uint64_t cycleclock_now() {
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
  uint32_t low, high;
  __asm volatile ("rdtsc" : "=a" (low), "=d" (high));
  return ((uint64_t)high << 32) | low;
#else
#error unsupported architecture
#endif
}

#endif  // CYCLECLOCK_H_
