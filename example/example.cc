#include "../bloom.h"

#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>


int main(int argc, char **argv) {
  size_t total_items = 1000000;
  struct bloom bf;
  double error = 0.01;
  if (bloom_init(&bf, total_items, error) != 0) 
  {
      std::cerr << "Init BF failed ...\n" << std::endl;
      exit(EXIT_FAILURE);
  }

  std::cout << "Capacity: " << bf.bits << std::endl;

  // Insert items to this bloom filter
  size_t num_inserted = 0;
  for (size_t i = 0; i < total_items; i++, num_inserted++) {
    if (bloom_add(&bf, (const void *)&i, sizeof(i)) == -1) {
      break;
    }
  }

  // Check if previously inserted items are in the filter, expected
  // true for all items
  for (size_t i = 0; i < num_inserted; i++) {
    assert(bloom_check(&bf, (const void*)&i, sizeof(i)));
  }

  // Check non-existing items, a few false positives expected
  size_t total_queries = 0;
  size_t false_queries = 0;
  for (size_t i = total_items; i < 2 * total_items; i++) {
    if (bloom_check(&bf, (const void*)&i, sizeof(i)) == 1) {
      false_queries++;
    }
    total_queries++;
  }

  // Output the measured false positive rate
  std::cout << "false positive rate is "
            << 100.0 * false_queries / total_queries << "%\n";

  bloom_free(&bf);
  return 0;
}