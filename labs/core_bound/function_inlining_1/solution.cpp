
#include "solution.h"
#include <algorithm>

void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), [](const S& a, const S&b) {
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  });

  // This version can also be used in C++20 and above:
  // std::ranges::sort(arr, [](const S& a, const S&b) {
  //   return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  // });
}
