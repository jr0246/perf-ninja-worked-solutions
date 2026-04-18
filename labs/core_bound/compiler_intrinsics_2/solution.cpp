#include "solution.hpp"

#include <cstdint>
#include <iostream>
#include <immintrin.h>

// Find the longest line in a file.
// Implementation uses ternary operator with a hope that compiler will
// turn it into a CMOV instruction.
// The code inside the inner loop is equivalent to:
/*
if (s == '\n') {
  longestLine = std::max(curLineLength, longestLine);
  curLineLength = 0;
} else {
  curLineLength++;
}*/
unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;
  int pos = 0;
  auto len = inputContents.size();

  if (len >= 32) {
    using v32i = __m256i;
    const v32i eol = _mm256_set1_epi8('\n');
    auto* ptr = inputContents.data();
    std::uint32_t curr_begin = 0;
    for (; pos + 32 < len; pos += 32) {
      v32i v = _mm256_loadu_si256(reinterpret_cast<const v32i*>(ptr));
      v32i v_mask = _mm256_cmpeq_epi8(v, eol);
      uint32_t mask = _mm256_movemask_epi8(v_mask);
      while (mask) {
        uint32_t chars = _tzcnt_u32(mask); // C++20 version: uint32_t chars = std::countr_zero(mask);
        uint32_t curr_len = (pos - curr_begin) + chars;
        if (pos < curr_begin) {
          // Case where our mask has picked up multiple '\n' in one chunk.
          // Therefore, the `curr_len` should just be the gap between adjacent '\n', which is `chars`.
          curr_len = chars;
        }

        curr_begin += curr_len + 1;
        longestLine = std::max(longestLine, curr_len);

        // Prepare bit-shift to remove current line of length `chars` plus the extra '\n' character.
        chars++;
        if (chars > 31) {
          break; // Do not shift a 32-bit integer by more than 31 bits; this is undefined behaviour.
        }
        mask >>= chars;
      }
      ptr += 32;
    }
  }

  for (; pos < len; pos++) {
    curLineLength = (inputContents[pos] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
