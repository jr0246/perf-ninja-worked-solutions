#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  static constexpr int TILE_SIZE = 16;
  for (int i = 0; i < size; i+= TILE_SIZE) {
    for (int j = 0; j < size; j+= TILE_SIZE) {
      for (int k = i; k < std::min(i + TILE_SIZE, size); k++) {
        for (int l = j; l < std::min(j + TILE_SIZE, size); l++) {
          out[k][l] = in[l][k];
        }
      }
    }
  }
  return out[0][size - 1];
}
