# Lab: loop_interchange_1

## Background:

The goal of this lab is to locate and adjust a suboptimal data access pattern caused by the ordering of loops.

Below is the performance of the original code (as seen with `perf stat ./lab`):

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           1261 ms         1261 ms            1

 Performance counter stats for './lab':

     1,264,770,134      task-clock                       #    0.999 CPUs utilized             
                70      context-switches                 #   55.346 /sec                      
                 2      cpu-migrations                   #    1.581 /sec                      
             1,369      page-faults                      #    1.082 K/sec                     
     8,124,445,942      instructions                     #    1.73  insn per cycle            
     4,692,293,039      cycles                           #    3.710 GHz                       
     1,160,302,252      branches                         #  917.402 M/sec                     
         3,006,154      branch-misses                    #    0.26% of all branches           

       1.266066116 seconds time elapsed
```

If we analyse the hotspots in `perf`, we notice it singles out a particular area in the code:



```
     22 │188:┌─→vmovss      xmm1,DWORD PTR [rcx]                                                                                                                                                                          ▒
        │    │for (int k = 0; k < N; k++) {                                                                                                                                                                               ▒
        │    │  add         rdx,0x640                                                                                                                                                                                     ▒
    650 │    │  add         rcx,0x4                                                                                                                                                                                       ▒
        │    │result[i][j] += a[i][k] * b[k][j];                                                                                                                                                                          ▒
   1384 │    │  vfmadd231ss xmm0,xmm1,DWORD PTR [rdx-0x640]                                                                                                                                                               ◆
    659 │    │  vmovss      DWORD PTR [r8],xmm0                                                                                                                                                                           ▒
        │    │for (int k = 0; k < N; k++) {                                                                                                                                                                               ▒
        │    ├──cmp         rsi,rdx                                                                                                                                                                                       ▒
      3 │    └──jne         188                                                                                                                                                                                           ▒
```

We notice it comes from the `multiply` function; specifically, the innermost loop:

```
// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      for (int k = 0; k < N; k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}
```

### Solution

The memory access pattern for the matrix `b` is suboptimal. The innermost loop iterates over `k`, which ends up jumping
from row to row in the matrix; in terms of memory, the processor has to jump around in memory to go from `b[k][j]` to
`b[k+1]j`. We want to rewrite this loop so that we keep the row fixed and iterate over each column in `a` and `b`, as
this will traverse memory contiguously.

To do this, we simply swap the iterators over `j` with those over `k`:

```
// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  for (int i = 0; i < N; i++) {
    for (int k = 0; k < N; k++) {
      for (int j = 0; j < N; j++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}
```

Here is the benchmark for the fixed code. As we notice, it is around ten times faster:

```
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
bench1/iterations:1        126 ms          126 ms            1

 Performance counter stats for './lab':

       133,631,700      task-clock                       #    0.992 CPUs utilized             
                 6      context-switches                 #   44.900 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             1,369      page-faults                      #   10.245 K/sec                     
       904,711,992      instructions                     #    1.91  insn per cycle            
       472,772,394      cycles                           #    3.538 GHz                       
       150,239,295      branches                         #    1.124 G/sec                     
            76,141      branch-misses                    #    0.05% of all branches           

       0.134680809 seconds time elapsed
```