# Lab: virtual_call_mispredict

## Hints

<details>
<summary><b>Hint 1:</b></summary>

What is it about the nature of the data in the `InstanceArray` (`std::vector<std::unique_ptr<BaseClass>>`)? Why is this
problematic?

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Think about the nature of polymorphism and the randomness. The CPU is unable to know which virtual method will be called
in advance in randomised data.

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Note that the hardware predictor performs best when the data are processed in a largely reliable and predictable manner.
Try seeing if there is a way to group the items that have the same virtual method call close together in the input data.

</details>



