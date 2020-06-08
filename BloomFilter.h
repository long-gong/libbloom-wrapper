/**
 * A C++ wrapper for libbloom, a simple and small bloom filter implementation in
 * C. APIs of our wrapper and implementations of those that the C implementation
 * does not provide are stolen (with minor changes) from the BloomFilter
 * implementation used in https://github.com/ArashPartow/bloom/blob/master/bloom_filter.hpp
 */

#ifndef BLOOM_FILTER_H_
#define BLOOM_FILTER_H_

#include "BitUtil.h"
#include "bloom.h"
#include <cmath>
#include <stdexcept>
#include <type_traits>


class BloomFilter {
 public:
  /* constructor */
  BloomFilter(size_t items, double error, unsigned int hashSeed = 0u): m_bf() {
    if (bloom_init(&m_bf, items, error) != 0) {
      throw std::runtime_error("Failed to initialize the bloom");
    }
    set_hash_seed(hashSeed);
#ifdef DEBUG
    printf("BF initialization succeeded!\n");
    bloom_print(&m_bf);
#endif
  }

  /** constructor: from an existing bitmap (storing using unsigned char).
   * 
   * This constructor is designed for using in the cases where you need to transmit a
   * Bloom filter between two remote hosts. For example, Alice (one host) creates
   * a Bloom filter and needs to send it to Bob (another host). In such cases, Alice
   * can simply send the arguments (of this constructor) to Bob, so Bob can re-construct the 
   * same Bloom filter with this constructor, 
   * 
   * NOTE THAT, the `raw_bf` will be moved, i.e., its memory will be re-used. If you can 
   * only copy, then the next constructor is your choice.
  */
  BloomFilter(size_t items, double error, unsigned char *&raw_bf,
              size_t len, unsigned int hashSeed = 0u) {
    m_bf.ready = 0;
    bloom_init_wo_allocation(&m_bf, items, error);
    set_hash_seed(hashSeed);
    if (len != m_bf.bytes) {
      throw std::runtime_error("Byte array sizes mismatch!");
    }
    // moving raw_bf
    m_bf.bf = raw_bf;
#ifdef DEBUG
    printf("Move byte array!\n");
#endif
    raw_bf = nullptr; // avoid doubly deleting
    m_bf.ready = 1;
  }

  /* constructor: from an existing bitmap (storing using unsigned char). */
  BloomFilter(size_t total_items, double error, const unsigned char *raw_bf,
              size_t len, unsigned int hashSeed = 0u) {
    if (bloom_init(&m_bf, total_items, error) != 0) {
      throw std::runtime_error("Failed to initialize the bloom");
    }
    set_hash_seed(hashSeed);
    // copy raw_bf
    try {
      set(raw_bf, len);
    } catch (const std::runtime_error& e) {
      // avoid memory leak
      bloom_free(&m_bf);
      throw std::runtime_error(e.what());
    }
  }

  /** Copy constructor */
  BloomFilter(const BloomFilter &other) {
    m_bf.ready = 0;
    *this = other;
  }

  /** copy assignment */
  inline BloomFilter &operator=(const BloomFilter &other) {
    if (this != &other) {
      size_t old_bits = (m_bf.ready == 0) ? 0 : m_bf.bits;
      m_bf.entries = other.m_bf.entries;
      m_bf.error = other.m_bf.error;
      m_bf.bits = other.m_bf.bits;
      m_bf.bytes = other.m_bf.bytes;
      m_bf.hashes = other.m_bf.hashes;
      m_bf.bpe = other.m_bf.bpe;
      if (old_bits != m_bf.bits) {
        m_bf.bf = (unsigned char *) realloc(m_bf.bf, m_bf.bits);
        if (m_bf.bf == nullptr) {
          throw std::runtime_error("Reallocating space failed.");
        }
      }
      std::copy(other.m_bf.bf, other.m_bf.bf + m_bf.bytes, m_bf.bf);
      m_bf.hashSeed = other.m_bf.hashSeed;
      m_bf.ready = other.m_bf.ready;
#ifdef COUNTING_SET_BITS_ON
      m_bf.num_set_bits = other.m_bf.num_set_bits;
#endif
    }
    return *this;
  }

  /* deconstructor */
  ~BloomFilter() { bloom_free(&m_bf); }

  /** Return the size of the "bit array" (NOTE: There is not an actual bit
   * array). */
  inline size_t size() const { return m_bf.bits; }

  /** Return the number of hash functions. */
  inline size_t num_hashes() const { return m_bf.hashes; }

  /** Return the hash seed (for reproducibility) */
  inline unsigned hash_seed() const { return m_bf.hashSeed; }

  /** Return the raw constant of bloom filter. */
  const unsigned char *bitmap() const { return m_bf.bf; }

  /** Return the size of the byte array. */
  inline size_t byte_size() const { return m_bf.bytes; }

  /** Return the number of bits that were set to 1
   * (NOTE: This API is
   * computationally expensive. If your application requires frequent accesses
   * to this API, then you are suggested to enable counting set bits. More
   * precisely, define a macro called COUNTING_SET_BITS_ON.)
   */
  inline size_t popcount() const {
#ifdef COUNTING_SET_BITS_ON
    return m_bf.num_set_bits;
#else
    size_t count = 0;
    size_t bytes = m_bf.bytes;
    size_t numInts = bytes / sizeof(uint64_t);
    size_t leftOverBytes = bytes % sizeof(uint64_t);
    auto *intPtr = reinterpret_cast<uint64_t *>(m_bf.bf);
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
  inline double effective_fpp() const {
    double one_minus_q = (double) popcount() / size();
    return std::pow(one_minus_q, num_hashes());
  }

  template<typename T>
  inline void add(const T *key, size_t len) {
    static_assert(std::is_integral<T>::value, "Integral Only");
    bloom_add_ns(&m_bf, (void *) key, len * sizeof(T));
  }

  inline void add(const char *key, size_t len) {
    bloom_add_ns(&m_bf, (void *) key, len);
  }

  inline void add(const unsigned char *key, size_t len) {
    bloom_add_ns(&m_bf, (void *) key, len);
  }

  template<typename T>
  inline void add(const T key) {
    static_assert(std::is_integral<T>::value, "Integral Only");
    bloom_add_ns(&m_bf, (void *) &key, sizeof(key));
  }

  inline void add(const std::string &key) {
    add(key.c_str(), key.size());
  }

  /** Check whether an object is contained. */
  template<typename T>
  inline bool contains(const T key) {
    static_assert(std::is_integral<T>::value, "Integral Only");
    return bloom_check_ns(&m_bf, (void *) &key, sizeof(key));
  }

  inline bool contains(const std::string& key) {
    return contains(key.c_str(), key.size());
  }

  /** Check whether an object is contained. */
  inline bool contains(const char *key, size_t len) {
    return bloom_check_ns(&m_bf, (void *) key, len);
  }

  /** Check whether an object is contained. */
  inline bool contains(const unsigned char *key, size_t len) {
    return bloom_check_ns(&m_bf, (void *) key, len);
  }

  template<typename T>
  inline bool contains(const T *key, size_t len) {
    static_assert(std::is_integral<T>::value, "Integral Only");
    return bloom_check_ns(&m_bf, (void *) &key, sizeof(key) * len);
  }

  /** Reset this bloom filter. */
  inline void reset() { bloom_reset(&m_bf); }

  /** Set this bloom filter with raw data `bf_data`. */
  inline void set(const unsigned char *bf_data, size_t size) {
#ifdef DEBUG
    printf("set()\n");
#endif
    if (size != byte_size())
      throw std::runtime_error("Byte array sizes mismatch!");
    std::copy(bf_data, bf_data + size, m_bf.bf);
  }

  /** Print this bloom filter. */
  inline void print() { bloom_print(&m_bf); }

 private:
  inline void set_hash_seed(unsigned seed) {
    if (seed > 0) m_bf.hashSeed = seed;
  }
  struct bloom m_bf{};
};

#endif // BLOOM_FILTER_H_