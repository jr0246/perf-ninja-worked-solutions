# Lab: mem_alignment_1

## Background:

This lab is all about memory alignment and the penalties that can be incurred do to the loading and storing of
incorrectly aligned data. The example given in the lab is matrix multiplication where the matrices are not correctly
aligned in memory.

The issue with using misaligned data is that data in a given row of a matrix may extend across two cache lines.
This forces any operations on these data to be split into two (i.e. split loads and stores), which leads to
inefficiencies as the two independent loads/stores must be reassembled in order to get the required value.

### Solution:

We can see how the misalignment can impact performance by looking at the output of running the lab binary:

```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
bench1/_63/iterations:30000       27.0 us         26.9 us        30000
bench1/_64/iterations:30000       18.8 us         18.8 us        30000
bench1/_65/iterations:30000       23.5 us         23.5 us        30000
bench1/_128                        180 us          180 us        15422
bench1/_256                       1869 us         1869 us         1504
bench1/_511/iterations:50        15535 us        15530 us           50
bench1/_512/iterations:50        16894 us        16892 us           50
bench1/_513/iterations:50        15924 us        15921 us           50
bench1/_1024                    200084 us       200038 us           10
```

Let's compare the `63`, `64`, and `65` column cases first. Running on x86, a cache line is 64 bytes long, meaning that
the matrices of width `63` and `65` are only slightly misaligned. We notice the aligned version is 30.3% faster than
`63` and 20% faster than `65`. Interestingly, we note the `512` case above seems to be running slightly lower than the
misaligned versions either side, so there are other factors at play.

So far, we have assumed that the first element of the matrix begins at the start of a cache line, but it may not be;
this could affect results, so let's align the beginning of the matrix and run the analysis again.

In `solution.h`, we should change the `using Matrix` definition to use the `AlignedVector` type defined in the header
file:

```c++
using Matrix = AlignedVector<float>;
```

Re-running the benchmark with the first element of each matrix correctly aligned to the beginning of a cache line:

```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
bench1/_63/iterations:30000       28.1 us         28.1 us        30000
bench1/_64/iterations:30000       19.5 us         19.5 us        30000
bench1/_65/iterations:30000       26.2 us         26.2 us        30000
bench1/_128                        151 us          151 us        16706
bench1/_256                       1398 us         1398 us         2004
bench1/_511/iterations:50        14134 us        14132 us           50
bench1/_512/iterations:50        12161 us        12161 us           50
bench1/_513/iterations:50        14286 us        14284 us           50
bench1/_1024                    124944 us       124938 us           22
```

Here, we see the expected performance increase in the `64` and `512` cases compared with the split versions either side.
This shows the importance of alignment of the start of the data.

How about the end? The misaligned column cases are still seeing noticeable performance degradation compared to those
whose columns are a multiple of the cache line size on x86 (64 bytes). In order to do this, we have to pad the matrices
with empty columns in order for their length to be equal to a multiple of the cache line size; or, more correctly, a
multiple of the number of elements of `float` that will fit on a cache line.

To do this, we create a function in `solution.cpp`:

```c++
int get_next_multiple_of_elems_in_cache_line(int N) {
  const auto y = ELEMS_PER_CACHE_LINE - 1;
  return N + y & ~y;
}
```

where `ELEMS_PER_CACHE_LINE` is defined in `solution.h` as follows:

```c++
inline constexpr int ELEMS_PER_CACHE_LINE = CACHELINE_SIZE / sizeof(float);
```

`CACHELINE_SIZE` is a constant provided by the existing code.

`get_next_multiple_of_cache_line` will return the first multiple of `ELEMS_PER_CACHE_LINE` that is greater than or
equal to `N`, where `N` will be the number of columns in our matrix.

Finally, we change the function `n_columns`:

```c++
int n_columns(int N) {
  return get_next_multiple(ELEMS_PER_CACHE_LINE, N);
}
```

Re-running the benchmarks gives us the following results:

```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
bench1/_63/iterations:30000       26.4 us         26.4 us        30000
bench1/_64/iterations:30000       18.8 us         18.8 us        30000
bench1/_65/iterations:30000       21.7 us         21.7 us        30000
bench1/_128                        155 us          155 us        18185
bench1/_256                       1463 us         1462 us         1910
bench1/_511/iterations:50        12625 us        12623 us           50
bench1/_512/iterations:50        12294 us        12290 us           50
bench1/_513/iterations:50        10436 us        10433 us           50
bench1/_1024                    127967 us       127944 us           22
```

While we don't notice too much of an impact for the smaller sized matrices, we see the performance of `511` now roughly
equal `512`, which makes sense as `511` now runs with a width of 512 bytes. Interestingly, `513` (now width 528 bytes)
appears to run faster than even the `512` case!