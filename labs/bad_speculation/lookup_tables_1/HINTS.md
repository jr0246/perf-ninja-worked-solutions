# Lab: lookup_tables_1

## Hints

<details>
<summary><b>Hint 1:</b></summary>

As always, begin by identifying the problematic section of code. `perf` is a great tool for this. You can also use
`perf stat` to confirm if the code is indeed suffering from a high rate of branch prediction errors (~10% is extremely
high).

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

While the branch taken by the `if` statements itself may not be predictable, have you noticed something about the
range of possible values `v` is being checked against? Is there anything predictable there? 

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

We know the ranges of `v` that should produce a given output. Use a hash table/array that maps these values of `v` to the
expected output. To handle values of `v` that are out of range, you can use `std::min(v, largest - 1)`. Note that the
compiler should produce branchless code for `std::min` when compiled with the correct optimisations.

</details>



