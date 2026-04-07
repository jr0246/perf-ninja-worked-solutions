# Lab: huge_pages_1

## Background:

This lab involves processing several arrays of doubles whose contents are in the tens of millions.
The sheer size of the arrays, as well as the random access of the elements, pose inherent issues
as these are not characteristics of a cache-friendly algorithm.

### Solution (Linux)

The problem can be alleviated slightly through the use of huge tables. Before explaining what these are
and how they work, I'll add a brief explanation of paging and virtual memory.

When a process (such as the program of our algorithm) is run by the operating system (OS),
the OS provides a virtualisation layer between the process and the machine's physical memory. This virtualisation
provides _virtual addresses_ to the process that it can use for reads and writes; hidden from the process, these virtual addresses are mapped
by the OS to real physical memory addresses.

Many OS platforms, such as Linux, implement the mapping between virtual and physical memory through the use
of pages. Pages can be thought of as tables whose entries describe the mapping from virtual to physical memory. When a
process performs a lookup for a memory address for the first time, the CPU goes to main memory (RAM) to retrieve it. Since
this is an extremely slow lookup, the results of these lookups are stored in a cache on the hardware chip. Each chip has a
cache associated with it on its memory-management unit (MMU), known as the Translation Lookaside Buffer (TLB).
The idea of the TLB is to provide extremely fast lookups of data that are regularly retrieved by the process.

When mapping a large memory space, such as on the order of megabytes as found in this program, the sheer number of pages
required to map the virtual memory to physical memory puts pressure on the TLB. The standard page size in Linux is 4 kB,
meaning thousands of pages would be needed to map all the memory used by the arrays in this algorithm. This exceeds the
number of entries that can be held in the TLB at a given time. Combined with the observation that this
algorithm performs data lookups in a random order, a very high number of expensive TLB cache misses is likely to occur.

By using huge pages (2MB), we can drastically increase the amount of memory mapped by one page (and thus one TLB entry)
by 512 times. This reduces the risk of TLB misses, as the cache will be able to store mappings for a much larger region 
of memory (as each page entry represents a region 512x larger than when using 4kB pages). This should boost performance.

When running the original code, we find the following performance:

```
-------------------------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------
Apply matrix-free operator       7.09 s          7.09 s             1 bytes_per_second=206.508Mi/s
```

With the updated code that uses huge pages (2MB):

```
-------------------------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------
Apply matrix-free operator       6.02 s          6.02 s             1 bytes_per_second=243.305Mi/s
```

We observe roughly 20% higher throughput of bytes and an overall reduction in runtime of around 1 second.

Below is the Linux code. It uses `mmap` and `munmap` to allocate and deallocate the memory. The `allocateDoublesArray`
function will allocate the array, and it will be backed by huge pages of size 2MB each. A `std::unique_ptr` is used with
a custom deleter, so that the allocated memory will be cleaned up automatically as soon as the object goes out of scope
in the program.

```c++
#include <sys/mman.h>
inline auto allocateDoublesArray(size_t size) {
  size_t total_array_size = size * sizeof(double);
  constexpr size_t page_2mb = 1UL << 21;
  auto pages_needed = total_array_size / page_2mb + (total_array_size % page_2mb != 0);
  size_t total_to_alloc = pages_needed * page_2mb;

  void* ptr = mmap(nullptr, total_to_alloc, PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);
  if (ptr == MAP_FAILED) {
    throw std::bad_alloc{};
  }
  madvise(ptr, total_to_alloc, MADV_HUGEPAGE);

  auto deleter = [total_to_alloc](double *p) { munmap(p, total_to_alloc); };

  return std::unique_ptr<double[], decltype(deleter)>(static_cast<double*>(ptr), std::move(deleter));
}
```