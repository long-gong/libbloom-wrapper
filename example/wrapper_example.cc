#include "BloomFilter.h"
#include <cassert>
#include <iostream>


void test_int(size_t num_entries, double error) {
  BloomFilter bf(num_entries, error);

  std::cout << "size: " << bf.size() << std::endl;

  for (size_t i = 0; i < 100; ++i) {
    assert(!bf.contain(i));
  }

  for (size_t i = 0; i < num_entries; ++i) {
    bf.insert(i);
  }

  for (size_t i = 0; i < num_entries; ++i) {
    assert(bf.contain(i));
  }

  size_t cf = 0;
  size_t ct = 0;

  for (size_t i = num_entries; i < 2 * num_entries; ++i) {
    if (bf.contain(i))
      cf++;
    ct++;
  }

  std::cout << "false positive rate: " << (double)cf / ct * 100 << "%\n";
}

int main() {

  size_t num_entries = 1000000;
  double error = 0.01;
  test_int(num_entries, error);
}