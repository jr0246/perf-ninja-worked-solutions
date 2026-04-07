# Lab: conditional_store_1

## Background:

The goal of this algorithm is to select the elements from `input` that fall within the specified range and populate
`output` with them.

Here is the original loop:
```c++
for (const auto item : input) {
    if ((lower <= item.first) && (item.first <= upper)) {
        output[count++] = item;
    }
}
```

Here is an example output obtained when running `perf stat ./lab` (`lab` being the built code), configured to run 10000
iterations:

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bench1/iterations:10000        286 us          285 us        10000

 Performance counter stats for './lab':

     2,868,419,549      task-clock                       #    0.999 CPUs utilized             
                52      context-switches                 #   18.128 /sec                      
                 1      cpu-migrations                   #    0.349 /sec                      
               528      page-faults                      #  184.073 /sec                      
     6,275,596,090      instructions                     #    0.56  insn per cycle            
    11,294,443,984      cycles                           #    3.938 GHz                       
     1,808,386,567      branches                         #  630.447 M/sec                     
       425,721,046      branch-misses                    #   23.54% of all branches           

       2.870168980 seconds time elapsed
```

We see a severely high level of branch mispredictions - 23.54% is extremely high. The algorithm needs to be rewritten to
fix this.

If we look at the predicate in the loop, we see that it is unpredictable because the data passed in i.e. `item.first`
is not determinate in any way.

```c++
if ((lower <= item.first) && (item.first <= upper)) {
    output[count++] = item;
}
```

What we need to do is change the loop such that we increment `count` based on the integer representation of the
predicate, `(lower <= item.first) && (item.first <= upper)`:

```
count += lower <= item.first && item.first <= upper;
```

In the event that the predicate is true, we advance the `count` index, and we then can populate the next element in the
output array. If `count` does not get incremented, it means that the element should not be selected and is safe to be
overwritten by a future iteration.

Let `K` be the total number of elements written to `output` by this algorithm. By the time the full loop has run,
the `K`th entry of the output array will either be valid, or it will be a value that was added pre-emptively,
but whose `item.first` was ultimately not in range, so it should be ignored.

We can use `count` to tell the caller of this function the total number of valid elements in the output array. If `count`
is not incremented on the last loop iteration, `count` will equal `K-1`, which will show the `K`th element will not be
counted.

The final loop should look like this:

```c++
for (const auto item : input) {
    output[count] = item;
    count += lower <= item.first && item.first <= upper;
}
```

Running `perf stat` on the lab a second time shows the algorithm is over 4x as fast as before (65.6 microseconds vs.
286 microseconds), and the percentage of branch misses is down to `0.03%`, which is a massive improvement.

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bench1/iterations:10000       65.6 us         65.6 us        10000

 Performance counter stats for './lab':

       664,023,567      task-clock                       #    0.998 CPUs utilized             
                25      context-switches                 #   37.649 /sec                      
                 1      cpu-migrations                   #    1.506 /sec                      
               528      page-faults                      #  795.153 /sec                      
     8,548,524,227      instructions                     #    3.34  insn per cycle            
     2,557,919,718      cycles                           #    3.852 GHz                       
       659,384,254      branches                         #  993.013 M/sec                     
           167,557      branch-misses                    #    0.03% of all branches           

       0.665149897 seconds time elapsed
```