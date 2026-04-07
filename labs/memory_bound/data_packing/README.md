# Lab: data_packing

## Background:

The struct `S` is suboptimally packed.

```c++
// FIXME: this data structure can be reduced in size
struct S {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

With the way it is above, there is extra padding in the following places:
1. 4 bytes between `int i` and `long long l`
2. 6 bytes between `short s` and `double d`
3. 7 bytes tail padding after `bool b` 

This results in 17 extra bytes of padding, resulting in `sizeof(S) = 40` bytes.

### Solution
The idea is to declare the largest data members first in the class to minimise its size:

```c++
struct S {
  long long l;
  double d;
  int i;
  short s;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

This struct only has padding at the end, of size 1, meaning `sizeof(S) = 24` here.

It is actually possible to do better than this by using bitfields.

If we inspect the input data in the lab, we realise that the largest value that could be passed to `int i` and `short s`
is `99`. Given that integers of width 7 bits can represent numbers 0 to 127, we can make `int i` 8 bits (1 byte)
rather than 32 (4 bytes), and `short s` can be reduced to 7 bits instead of 16.

By extension, as these two numbers are multiplied to find the value of `l`, we know `l` will not exceed 10,000.
`l` can therefore be reduced to a 16-bit width. `double` is also too large a data type for the ramge we are considering
for division, so we can replace it with a `float`.

The version with bitfields:

```c++
struct S {
  long long l:16;
  float d;
  int i:8;
  short s:7;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

This eventually produces an object whose `sizeof(S) = 16` bytes, with only one byte of padding after `bool b`.