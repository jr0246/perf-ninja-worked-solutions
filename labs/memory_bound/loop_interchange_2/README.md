# Lab: loop_interchange_2

## Background:

This lab involves an algorithm that applies Gaussian blur to a supplied input image.
Its performance is hindered by a hot spot that needs to be rewritten.

We can perform a top-down analysis of the code using [pmu-tools](https://github.com/andikleen/pmu-tools), which is a suite of tools
built on top of Linux `perf` to run performance analysis on Intel CPUs.

We can run the `toplev.py` script (provided Python is installed) in `pmu-tools` on this lab as follows:

`toplev.py --core S0-C0 --run-sample -l2 -v --no-desc taskset -c 0 ./lab ARG1 ARG2`

where `ARG1` and `ARG2` are the paths to the input and output `.pgm` files, respectively.

(To use the examples in the repository, they are `${CMAKE_CURRENT_SOURCE_DIR}/pexels-pixabay-434334.pbm` and `output.pgm`, which will be in the `build` directory
alongside `lab` after `lab` is run).

Part of the output shows us the following:

```
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
bench1/iterations:5        290 ms          290 ms            5
...
C0    FE               Frontend_Bound                      % Slots                        3.5  < [ 8.0%]
C0    BAD              Bad_Speculation                     % Slots                        0.3  < [ 8.0%]
C0    BE               Backend_Bound                       % Slots                       74.5    [ 8.0%]
C0    RET              Retiring                            % Slots                       21.6  < [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Latency        % Slots                        1.9  < [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Bandwidth      % Slots                        1.1  < [ 8.0%]
C0    BAD              Bad_Speculation.Branch_Mispredicts  % Slots                        0.2  < [ 8.0%]
C0    BAD              Bad_Speculation.Machine_Clears      % Slots                        0.1  < [ 8.0%]
C0    BE/Mem           Backend_Bound.Memory_Bound          % Slots                       49.7    [ 8.0%]<==
C0    BE/Core          Backend_Bound.Core_Bound            % Slots                       24.8    [ 8.0%]
C0    RET              Retiring.Light_Operations           % Slots                       20.9  < [ 8.0%]
C0    RET              Retiring.Heavy_Operations           % Slots                        0.4  < [ 8.0%]
C0-T0 MUX                                                  %                              8.00  
C0-T1 MUX                                                  %                              8.00  
Run toplev --describe Memory_Bound^ to get more information on bottleneck```
```

This output highlights that the code is primarily memory bound, where almost 50% of resources are
held up simply waiting for data to be available.

If we run `perf report`, we can locate the problematic line of code:

```c++
dot += input[(r - radius + i) * width + c] * kernel[i];
```

This is from line 37 in `solution.cpp`. The full loop:

```c++
for (int c = 0; c < width; c++) {
  ...
  // Middle part of computations with full kernel
  for (int r = radius; r < height - radius; r++) {
      // Accumulation
      int dot = 0;
      for (int i = 0; i < radius + 1 + radius; i++) {
          dot += input[(r - radius + i) * width + c] * kernel[i];
      }

      // Fast shift instead of division
      int value = (dot + rounding) >> shift;
      output[r * width + c] = static_cast<uint8_t>(value);
  }
}
```

As we see, when the inner loop over `i` advances, there is a jump of `width` elements in the next `input` lookup.
This is not cache-friendly at all; jumping around in memory in this way does not make good use of the surrounding
elements that would also be loaded in a cache line (or that may also be in L1 cache). These large strides will lead
to many expensive cache misses and degraded performance.

### Solution

We need to rearrange the loops so that we iterate over `c` _inside_ the loop of `i`. As it
is currently written, this is not trivial as there are dependencies between loop iterations; the value
`dot` is incremented to on the `K`th iteration depends on its value from the `K-1`th iteration, and so on.

To break this dependency, we therefore need to expand `dot` to be an `int` array of length `width`.

First, we extract this section of the code from top-level loop over `c` in this function:

```c++
  for (int c = 0; c < width; c++) {
    // Top part of line, partial kernel
    ...
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }
  } // *** <--- STOP `c` HERE ***

  // Middle part of computations with full kernel
  ...

  for (int c = 0; c < width; c++) { // <--- *** RESTART `c` HERE ***
    // Bottom part of line, partial kernel
```

We then change `dot` to be an array of length `width`, so we can accumulate for a given `c` (as in the previous code),
but perform the `c` loop _inside_ the `i` loop in order to improve the memory access pattern:

```c++
  for (int r = radius; r < height - radius; r++) {
    int dot[width];
    for (int c = 0; c < width; c++) {
      dot[c] = 0; // <- INITIALISATION
    }
    // Accumulation
    for (int i = 0; i < radius + 1 + radius; i++) {
      for (int c = 0; c < width; c++) {
        // <- LOOP OVER `c` INSIDE `i`; sequential accesses are cache-friendly
        dot[c] += input[(r - radius + i) * width + c] * kernel[i];
      }
    }
    // <- PUT `c`th loop value into `output` as before
    for (int c = 0; c < width; c++) {
      // Fast shift instead of division
      int value = (dot[c] + rounding) >> shift;
      output[r * width + c] = static_cast<uint8_t>(value);
    }
  }
```

Running the benchmark again after these changes shows a remarkable speedup (from 290ms to 71.4ms), with memory bound
accesses down to only 5.4% of the resource usage. The code is now mainly bound by the computations the algorithm just has to do.

```
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
bench1/iterations:5       71.4 ms         71.0 ms            5
...
C0    FE               Frontend_Bound                      % Slots                        5.4  < [ 8.0%]
C0    BAD              Bad_Speculation                     % Slots                        0.7  < [ 8.0%]
C0    BE               Backend_Bound                       % Slots                       27.4    [ 8.0%]
C0    RET              Retiring                            % Slots                       66.6  < [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Latency        % Slots                        2.8  < [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Bandwidth      % Slots                        2.1  < [ 8.0%]
C0    BAD              Bad_Speculation.Branch_Mispredicts  % Slots                        0.5  < [ 8.0%]
C0    BAD              Bad_Speculation.Machine_Clears      % Slots                        0.2  < [ 8.0%]
C0    BE/Mem           Backend_Bound.Memory_Bound          % Slots                        5.4  < [ 8.0%]
C0    BE/Core          Backend_Bound.Core_Bound            % Slots                       22.0    [ 8.0%]<==
C0    RET              Retiring.Light_Operations           % Slots                       64.5    [ 8.0%]
C0    RET              Retiring.Heavy_Operations           % Slots                        3.3  < [ 8.0%]
C0-T0 MUX                                                  %                              8.00  
C0-T1 MUX                                                  %                              8.00 
```