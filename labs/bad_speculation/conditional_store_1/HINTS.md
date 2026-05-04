# Lab: conditional_store_1

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Begin by understanding why the code in the loop is hard to predict. Is there something about the nature of the input
data that can explain this?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

The inputs are random, so the hardware will not be able to predict if `item` will satisfy
`(lower <= item.first) && (item.first <= upper)`. Is there a way we could rewrite this loop so that it does not rely on an
`if` statement?
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Try assuming that `item` fits the criteria and write it directly to `output[count]` at the beginning of each iteration.
What would we then need to do to `count` to ensure we only keep the `item`s that should be in `output` in following
loop iterations? Can we achieve these updates to `count` without using an `if` statement?
</details>



