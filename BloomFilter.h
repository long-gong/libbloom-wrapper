/**
 * A C++ wrapper for libbloom, a simple and small bloom filter implementation in
 * C. APIs of our wrapper and implementations of those that the C implementation
 * does not provide are stolen (with minor changes) from the BloomFilter
 * implementaion used in https://github.com/bcgsc/abyss
 */

#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_

#include "BitUtil.h"
#include "bloom.h"
#include <cmath>
#include <stdexcept>

class BloomFilter {
public:
  /* constructor */
  BloomFilter(size_t total_items, double error, unsigned int hashSeed = 0u) {
    if (error <= 0 || error > 1) {
      throw std::logic_error("Invalid argument for `error`, it should be a "
                             "number between 0 and 1.");
    }
    // NOTE: we allow empty bloom filter, since one of our applications
    // could create an empty bloom filter
    if (total_items > 0) {
      if (bloom_init(&m_bf, total_items, error) != 0) {
        throw std::runtime_error("Failed to initialize the bloom");
      }
      if (hashSeed > 0) {
        m_bf.hashSeed = hashSeed;
      }
    }
  }

  /* constructor */
  BloomFilter(size_t total_items, double error, unsigned char *&raw_bf,
              size_t len, unsigned int hashSeed = 0u) {
    if (error <= 0 || error > 1) {
      throw std::logic_error("Invalid argument for `error`, it should be a "
                             "number between 0 and 1.");
    }
    // NOTE: we allow empty bloom filter, since one of our applications
    // could create an empty bloom filter
    if (total_items > 0) {
      bloom_init_wo_allocation(&m_bf, total_items, error);
      if (hashSeed > 0) {
        m_bf.hashSeed = hashSeed;
      }
    }

    if (len != m_bf.bits) {
      throw std::logic_error("Bit array sizes do not match!");
    }
    // moving raw_bf
    m_bf.bf = std::move(raw_bf);

    raw_bf = nullptr; // avoid doubly deleting
  }

  /* constructor */
  BloomFilter(size_t total_items, double error, const unsigned char *raw_bf,
              size_t len, unsigned int hashSeed = 0u) {
    if (error <= 0 || error > 1) {
      throw std::logic_error("Invalid argument for `error`, it should be a "
                             "number between 0 and 1.");
    }
    // NOTE: we allow empty bloom filter, since one of our applications
    // could create an empty bloom filter
    if (total_items > 0) {
      if (bloom_init(&m_bf, total_items, error) != 0) {
        throw std::runtime_error("Failed to initialize the bloom");
      }
      if (hashSeed > 0) {
        m_bf.hashSeed = hashSeed;
      }
    }

    // copy raw_bf
    set(raw_bf, len);
  }

  /* deconstructor */
  ~BloomFilter() { bloom_free(&m_bf); }

  /** Return the size of the "bit array" (NOTE: There is not an actual bit
   * array). */
  size_t size() const { return m_bf.bits; }

  /** Return the number of hash functions. */
  size_t num_hashes() const { return m_bf.hashes; }

  /** Return the hash seed (for reproducibility) */
  size_t hash_seed() const { return m_bf.hashSeed; }

  /** Return the raw constant of bloom filter. */
  const unsigned char *const data() const { return m_bf.bf; }

  /** Return the size of the byte array. */
  size_t byte_size() const { return m_bf.bytes; }

  /** Return the number of bits that were set to 1 **/
  size_t num_set_bits() const {
#ifdef COUNTING_SET_BITS_ON
    return m_bf.num_set_bits;
#else
    size_t count = 0;
    size_t bytes = m_bf.bytes;
    size_t numInts = bytes / sizeof(uint64_t);
    size_t leftOverBytes = bytes % sizeof(uint64_t);
    uint64_t *intPtr = reinterpret_cast<uint64_t *>(m_bf.bf);
    for (size_t i = 0; i < numInts; i++) {
      count += ::popcount(intPtr[i]);
    }
    for (size_t i = bytes - leftOverBytes; i < bytes; ++i) {
      unsigned char c = m_bf.bf[i];
      for (unsigned j = 0; j < 8; ++j) {
        unsigned int mask = 1u << j;
        if (c & mask)
          count++;
      }
    }
    return count;
#endif
  }

  /** Return the estimated false positive rate (NOTE: This API is
   * computationally expensive. If your application requires frequent accesses
   * to this API, then you are suggested to enable counting set bits. More
   * precisely, define a macro called COUNTING_SET_BITS_ON.)*/
  double fpr() const {
    double one_minus_q = (double)num_set_bits() / size();
    return std::pow(one_minus_q, num_hashes());
  }

  /** Add the object to this set. */
  template <typename elementary_key_type>
  void insert(const elementary_key_type &key) {
    bloom_add(&m_bf, &key, sizeof(key));
  }

  /** Add the object to this set. */
  void insert(const char *key, size_t len) { bloom_add(&m_bf, key, len); }

  /** Check whether an object is contained. */
  template <typename elementary_key_type>
  bool contain(const elementary_key_type &key) {
    return bloom_check(&m_bf, &key, sizeof(key));
  }

  /** Check whether an object is contained. */
  bool contain(const char *key, size_t len) {
    return bloom_check(&m_bf, key, len);
  }

  /** Reset this bloom filter. */
  void reset() { bloom_reset(&m_bf); }

  /** Set this bloom filter with raw data `bf_data`. */
  void set(const unsigned char *bf_data, size_t size) {
    if (size != this->size())
      throw std::logic_error("Array size not match!");
    std::copy(bf_data, bf_data + size, m_bf.bf);
  }

  /** Print this bloom filter. */
  void print() { bloom_print(&m_bf); }

private:
  struct bloom m_bf;
};

template <> void BloomFilter::insert(const std::string &key) {
  bloom_add(&m_bf, key.c_str(), key.size());
}

template <> bool BloomFilter::contain(const std::string &key) {
  return bloom_check(&m_bf, key.c_str(), key.size());
}

#endif // BLOOMFILTER_H_