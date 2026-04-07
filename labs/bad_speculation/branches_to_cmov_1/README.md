# Lab: branches_to_cmov_1

## Background:

The aim of this lab is to replace unpredictable branches with conditional moves.
A conditional move `cmov` is not necessarily always faster than code with branches, but in this case, where the branches
are not predictable, we should expect (and do observe) a performance increase.

### Solution:

The main section of branches exists here:

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

The assembly code output for the above can be viewed [here](https://godbolt.org/z/Gdza4dzze). 

_(Note: The above is a code snippet compiled without optimisations for illustrative purposes; no test data are provided for `future` and `current`, so `-O3` optimises most of the assembly instructions out as it notices the value of `future[i][j]` will remain 0, i.e., unchanged, unless `aliveNeighbours == 3`)._

From this, we can note three distinct final outcomes for the value of `future[i][j]`:

1. `future[i][j] = 0` - if `aliveNeighbours` is any value other than 2 or 3.
2. `future[i][j] = current[i][j]` - remains the same if `aliveNeighbours` is equal to 2.
3. `future[i][j] = 1` - if `aliveNeighbours` equals 3.

This shows that we can begin with assuming `future[i][j]` will equal 0, and write code that changes the outcome only for
`aliveNeighbours == 2` or `aliveNeighbours == 3`. This will simplify the branches.

```c++
// Implementing the Rules of Life:
int cell_value = 0;
if (aliveNeighbours == 2) {
    cell_value = current[i][j];
} else if (aliveNeighbours == 3) {
    cell_value = 1;
}
future[i][j] = cell_value;
```

This is an improvement, but we still have not eliminated the branches entirely.

To do this, we need some help from compiler intrinsics. Given the fact that `aliveNeighbours` is not predictable, the performance of the code as it is will suffer from major branch misprediction penalties.
On `x86`, we use the built-in `__builtin_unpredictable()` on Clang (versions 17+) to indicate that this branch cannot be predicted by hardware prediction methods. This will help the compiler produce better assembly code.

```c++
// Implementing the Rules of Life:
int cell_value = 0;
if (__builtin_unpredictable(aliveNeighbours == 2)) {
    cell_value = current[i][j];
} else if (__builtin_unpredictable(aliveNeighbours == 3)) {
    cell_value = 1;
}
future[i][j] = cell_value;
```