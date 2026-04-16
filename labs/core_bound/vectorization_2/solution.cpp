#include "solution.hpp"


uint16_t checksum(const Blob& blob) {
  constexpr std::size_t two_pow_16 = 1 << 16;
  uint32_t acc = 0, prev = 0;
  for (std::size_t i = 0; i < N; i += two_pow_16) {
    for (std::size_t j = i; j < i + two_pow_16 && j < N; j++) {
      acc += blob[j];
    }
    if (acc < prev) {
      acc++;
    }
    prev = acc;
  }
  uint16_t top = acc >> 16;
  uint16_t bottom = acc & two_pow_16 - 1;
  uint16_t ans = top + bottom;
  ans += ans < top;
  return ans;
}
