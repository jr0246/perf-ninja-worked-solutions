# Lab: branches_to_cmov_1

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Have you found the part of the code where there may be a lot of branch mispredictions? Are there any sections where you
would find it difficult to say where the code is likely to flow based on random inputs?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

A good place to start would be to identify statements involving `if` or `switch` whose outcomes cannot be easily
predicted due to the nature of the input data. These places are likely to cause problems for the hardware branch
predictor. You can confirm your thoughts by profiling with something like `perf record` and viewing the code it
highlights as the cause of branch mispredictions.
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

The main code section to focus on is here:

```c++
// Implementing the Rules of Life:
switch(aliveNeighbours) {
    // 1. Cell is lonely and dies
    case 0:
    case 1:
        future[i][j] = 0;
        break;                   
    // 2. Remains the same
    case 2:
        future[i][j] = current[i][j];
        break;
    // 3. A new cell is born
    case 3:
        future[i][j] = 1;
        break;
    // 4. Cell dies due to over population
    default:
        future[i][j] = 0;
}
```

Notice that many of the branches can be removed by noting that `future[i][j]` will always be `0` except if
`aliveNeighbours` is `2` or `3`. After that, we can use `__builtin_unpredictable()` (if using x86 and Clang 17+) to
give a hint to the compiler that these branches will be tricky to predict. In turn, this built-in can help the compiler
produce more appropriate assembly code to reduce the impact of branching i.e. by generating `cmov` instructions.
</details>


