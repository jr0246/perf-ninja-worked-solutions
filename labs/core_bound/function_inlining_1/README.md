# Lab: function_inlining_1

## Background:

Function inlining is one of the most important compiler optimisations for performance. A normal function call incurs
some overhead when called, particularly in the _prologue_ and _epilogue_. The function prologue is code that prepares
the stack and registers required for said function, while the epilogue performs the opposite by restoring the stack and
registers to the state they were in before the function call.

Function inlining can help us minimise these overheads. When a function is inlined, we can think of the function's code
being put directly in place of where the function call was originally written. If a function can be inlined, we will
often observe major performance improvements.

If we look at the original code in the lab:

```c++
void solution(std::array<S, N> &arr) {
  qsort(arr.data(), arr.size(), sizeof(S), compare);
}
```

We notice we are making a call to `qsort`, which is a quicksort algorithm from C. The trouble here is that this function
is in a compiled C library, which makes is nigh-on impossible for the compiler to "see" into it and make the
optimisations required to inline it.

Let's note a benchmark performance score before moving on to the solution:

```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:5000        760 us          760 us         5000
```

### Solution

In this lab, the better approach would be to use the `std::sort` (or `std::ranges::sort` in C++20 and above) from the
standard library. Modern C++ compilers find it very easy to inline standard library code.

Let's rewrite it using the standard library and a lambda function for the comparator:

```c++
void solution(std::array<S, N> &arr) {
  std::sort(arr.begin(), arr.end(), [](const S& a, const S&b) {
    return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  });

  // This version can also be used in C++20 and above:
  // std::ranges::sort(arr.begin(), arr.end(), [](const S& a, const S&b) {
  //   return a.key1 < b.key1 || (a.key1 == b.key1 && a.key2 < b.key2);
  // });
}
```

Running this code provides us with a better performance profile (518us vs. 760us):

```c++
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:5000        518 us          518 us         5000
```