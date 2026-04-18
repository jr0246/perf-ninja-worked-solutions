
#include "solution.h"

#include <emmintrin.h>
#include <smmintrin.h>
#include <memory>

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  // 1. left border - time spend in this loop can be ignored, no need to
  // optimize it
  for (int i = 0; i < std::min<int>(size, radius); ++i) {
    currentSum += input[i];
  }

  int limit = std::min(radius + 1, size - radius);
  for (pos = 0; pos < limit; ++pos) {
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 2. main loop.
  limit = size - radius;
  const uint8_t* subtract_ptr = input.data() + pos - radius - 1;
  const uint8_t* add_ptr = input.data() + pos + radius;
  const uint16_t* output_ptr = output.data() + pos;

  using v8i = __m128i;
  v8i current = _mm_set1_epi16(currentSum);

  int i = 0;
  for (; i + 7 + pos < limit; i += 8) {
    v8i sub_pckd_8 = _mm_loadu_si64(subtract_ptr + i);
    v8i add_pckd_8 = _mm_loadu_si64(add_ptr + i);
    v8i sub_ext_16 = _mm_cvtepu8_epi16(sub_pckd_8);
    v8i add_ext_16 = _mm_cvtepu8_epi16(add_pckd_8);

    v8i diff = _mm_sub_epi16(add_ext_16, sub_ext_16);

    v8i delta = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 4));
    delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 8));

    v8i result = _mm_add_epi16(delta, current);
    _mm_storeu_si128((v8i*)(output_ptr + i), result);

    currentSum = static_cast<uint16_t>(_mm_extract_epi16(result, 7));
    current = _mm_set1_epi16(currentSum);
  }
  pos += i;

  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  for (; pos < limit; pos++) {
    output[pos] = currentSum;
  }

  // 4. right border - time spend in this loop can be ignored, no need to
  // optimize it
  for (; pos < size; ++pos) {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
