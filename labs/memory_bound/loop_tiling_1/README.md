# Lab: loop_tiling_1

## Hints

<details>
<summary><b>Hint 1:</b></summary>

When transposing a matrix, if we access the elements of the original matrix in row major order, then we have to access
the transposed object in column major order, which is not generally very cache-friendly. Is there something we can do
to lessen the impact?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

While reading from element `[j, i]` of `in`, we pull in the whole cache line that said element lives on into our cache. 
However, we disregard these colocated elements as we traverse `in` in column major order (for a given column,
we iterate over all 2000 rows). So, if we start on `[0,0]`, we iterate up to `[2000, 0]` before we get to `[0, 1]`.
This memory at `in[0][1]` is likely to have been evicted from the cache by then, even though this memory address
would have been there due to our access of `in[0][0]`. Can we refactor the loop to reduce the risk of us evicting useful
data too often from our cache?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

The key is to break up the processing of `in` and `out` into smaller sized tiles such that the *total* memory associated with
both can fit entirely into the L1 (or even L2) cache. In this way, we can minimise the number of expensive cache misses
encountered while transposing the data.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The lab here transposes a large 2000x2000 matrix. When we look at the original loop, we can make some observations:

```c++
for (int i = 0; i < size; i++) {
  for (int j = 0; j < size; j++) {
    out[i][j] = in[j][i];
  }
}
```

Matrix `in` is accessed in column-major order (i.e., `(0,0)` -> `(1,0)` -> ... -> `(2000,0)` -> `(0,1)` -> ...). Due to this
access pattern and the size of the array, the code does not benefit from the way memory is loaded into the cache.

When the code loads the element at (0,0), the next few elements in the 0th row (such as `(0,1)`, `(0,2)` etc.) will also
be loaded as part of the cache line in which they all reside in memory. However, as we traverse in column-major order,
we never look at these next elements. By the time we get back to the 0th row (after 2000 iterations), and try to access
`(0,1)`, it is almost certain that we will experience a cache miss due to the eviction of the original data from the cache
by the intermediate lookups.

An example lookup time for 500 iterations with the original code is as follows:

```
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
bench1/iterations:500       16.8 ms         16.8 ms          500
```

### Solution

One way to address this cache eviction problem is to break the larger matrix down into tiles, or blocks, such that
the total size of both blocks is small enough to fit in the L1 cache (and perhaps L2 cache as well). This approach is
highly platform dependent, and ultimately needs to be tuned to a given machine and cache size.

On my machine, I tested 64x64, 32x32, and 16x16 tiles. I observed the best results when subdividing the matrix into
16x16 sized chunks and performing the transposition as follows:

```
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
```

The new benchmark shows a time that is almost three times as fast as the original code:

```
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
bench1/iterations:500       5.72 ms         5.72 ms          500
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/memory_bound/loop_tiling_1/solution.cpp)

</details>