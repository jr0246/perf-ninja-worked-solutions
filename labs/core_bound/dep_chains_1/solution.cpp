#include "solution.hpp"
#include <array>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

// Task: lookup all the values from l2 in l1.
// For every found value, find the sum of its digits.
// Return the sum of all digits in every found number.
// Both lists have no duplicates and elements placed in *random* order.
// Do NOT sort any of the lists. Do NOT store elements in a hash_map/sets.

// Hint: Traversing a linked list is a long data dependency chain:
//       to get the node N+1 you need to retrieve the node N first.
//       Think how you can execute multiple dependency chains in parallel.
template<int N>
unsigned solution(List *l1, List *l2) {
  List* head1 = l1;
  int length1 = 0;
  while (head1 != nullptr) {
    length1++;
    head1 = head1->next;
  }
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

  return retVal;
}

unsigned solution(List *l1, List *l2) {
  return solution<4>(l1, l2);
}