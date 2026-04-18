# Lab: dep_chains_1

## Background:

Dependency chains in code can lead to bottlenecks in performance. By dependency chain, we refer to sections of code
that must be done before a new task can be completed; the new task has a _dependency_ on the prior chain. This can be
problematic as it prevents the ability to parallelise workflows.

Let's consider the problem in the lab. We have a quadratic algorithm in which we take every node in a linked list and
search for a node with the same value in another list.

The dependency chain in this problem is fairly clear; the `n`th node in the linked list cannot be accessed before the
`n-1`th element, which in turn requires the `n-2`th element, and so on.

Here is a sample performance report from the original code run:
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           85.5 ms         85.4 ms           33
```

### Solution

As there is no way to break the dependency chains here, we have to think of a more efficient strategy. Given that we
compare every node in list A to the nodes in list B, a better approach would be to compare a _group_ of nodes from A
during the visit of every node of B. By doing this, we take a set of nodes from A and loop over its entries at every
node of B. In this way, we can reduce the impact of the dependency chains by reducing the time interval between when
we compare nodes `N` and `N+1` from list A with the linked list B.

We can create a templated version of the solution function that takes as its parameter the number of nodes from A to
process per loop iteration:

```c++
template<int N>
unsigned solution(List *l1, List *l2) {
    ...
}
```

First, we count the length of list `l1`:

```c++
List* head1 = l1;
int length1 = 0;
while (head1 != nullptr) {
    length1++;
    head1 = head1->next;
}
```

Then, we compare the chunks of size `N` to each node in `l2`, shifting prematurely to the next chunk of size `N` if we
find all `N` values in `l2`:

```c++
  unsigned retVal = 0;
  List* head2 = l2;

  // Process in chunks
  for (int i = 0; i < length1 / N; i++) {
    std::array<unsigned, N> arr{};
    for (int j = 0; j < N; j++) {
      arr[j] = l1->value;
      l1 = l1->next;
    }
    int seen = 0;
    l2 = head2;
    while (l2) {
      int v = l2->value;
      for (int j = 0; j < N; j++) {
        if (arr[j] == v) {
          retVal += getSumOfDigits(v);
          if (++seen == N) {
            break;
          }
        }
      }
      l2 = l2->next;
    }
  }
```

For completeness, we have to account for any remaining elements of `l1` that were not handled in the prior blocks of `N`:

```c++
  // process any remaining elements in l1 (<N elements)
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }
```

What is a sensible size of blocks of nodes to search? That is up to you to try on your machine, but noticing that the
linked lists are allocated in memory via an arena allocator, meaning the elements are adjacent to one another in
contiguous memory. Perhaps we load one cache line at a time, so the load of the first node from memory will also pull a
number of adjacent nodes at the same time. The struct `Line` has size 16 (a pointer of size 8, and an unsigned int of
size 8):

```c++
struct List {
  List *next;
  unsigned value;
};
```

On x86 systems, a cache line is 64 bytes wide; therefore, a logical place to start would be to work with blocks of 4
Line pointers at a time, as these point to a region in memory equal to a cache line.

Using `N = 4`, the benchmark shows a marked improvement - twice the number of iterations in half the time!

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           34.7 ms         34.7 ms           78
```