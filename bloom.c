/*
 *  Copyright (c) 2012-2019, Jyri J. Virkki
 *  All rights reserved.
 *
 *  This file is under BSD license. See LICENSE file.
 */

/*
 * Refer to bloom.h for documentation on the public interfaces.
 */

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bloom.h"

#ifndef HASH_FN
#if defined(USE_XXHASH)
#include <xxhash.h>
#define HASH_FN(key, len, seed) XXH64(key, len, seed)
#elif defined(USE_WYHASH)
#include <wyhash.h>
#define HASH_FN(key, len, seed) wyhash(key, len, seed, _wyp)
#else
#include "murmurhash2.h"
#define HASH_FN(key, len, seed) murmurhash2(key, len, seed)
#endif
#endif

#define MAKESTRING(n) STRING(n)
#define STRING(n) #n

inline static int test_bit_set_bit(unsigned char *buf, size_t x,
                                   int set_bit) {
  size_t byte = (size_t) x >> 3u;
  unsigned char c = buf[byte]; // expensive memory access
  unsigned int mask = 1u << (x % 8);

  if (c & mask) {
    return 1;
  } else {
    if (set_bit) {
      buf[byte] = c | mask;
    }
    return 0;
  }
}

inline static int test_bit(const unsigned char *buf, size_t x) {
  size_t byte = x >> 3u;
  unsigned char c = buf[byte]; // expensive memory access
  unsigned int mask = 1u << (x & 0x7lu);
  return (c & mask) != 0;
}

inline static void set_bit(unsigned char *buf, size_t x) {
  size_t byte = x >> 3u;
  unsigned int mask = 1u << (x & 0x7lu);
#ifdef  COUNTING_SET_BITS_ON
  unsigned char c = buf[byte]; // expensive memory access
  if (c & mask == 0)  bloom.num_set_bits ++;
#endif
  buf[byte] |= mask;
}

static int bloom_check_add(struct bloom *bloom, const void *buffer, int len,
                           int add) {
  if (bloom->ready == 0) {
    printf("bloom at %p not initialized!\n", (void *) bloom);
    return -1;
  }

  int hits = 0;
  register size_t a = HASH_FN(buffer, len, bloom->hashSeed);
  register size_t b = HASH_FN(buffer, len, a);
  register size_t x;
  register unsigned int i;

  for (i = 0; i < bloom->hashes; i++) {
    x = (a + i * b) % bloom->bits;
    if (test_bit_set_bit(bloom->bf, x, add)) {
      hits++;
    } else if (!add) {
      // Don't care about the presence of all the bits. Just our own.
      return 0;
    }
  }
#ifdef COUNTING_SET_BITS_ON
  if (add)
    bloom.num_set_bits += bloom->hashes - hits;
#endif

  if (hits == bloom->hashes) {
    return 1; // 1 == element already in (or collision)
  }

  return 0;
}

int bloom_init_size(struct bloom *bloom, size_t entries, double error,
                    unsigned int cache_size) {
  return bloom_init(bloom, entries, error);
}

// added by Long
void bloom_init_wo_allocation(struct bloom *bloom, size_t entries,
                              double error) {
  bloom->entries = entries;
  bloom->error = error;

  double num = log(bloom->error);
  double denom = 0.480453013918201; // ln(2)^2
  bloom->bpe = -(num / denom);

  double dentries = (double) entries;
  bloom->bits = (size_t) ceil(dentries * bloom->bpe);

  if (bloom->bits % 8) {
    bloom->bytes = (bloom->bits / 8) + 1;
  } else {
    bloom->bytes = bloom->bits / 8;
  }

  bloom->hashes = (int) ceil(0.693147180559945 * bloom->bpe); // ln(2)

  bloom->hashSeed = 0x9747b28c;
#ifdef COUNTING_SET_BITS_ON
  bloom.num_set_bits = 0;
#endif
}

int bloom_init(struct bloom *bloom, size_t entries, double error) {
#ifdef DEBUG
  printf("entries = %lu, error = %.8f\n", entries, error);
#endif
  bloom->ready = 0;
  if (!(entries > 0 && error > 0 && error < 1.0))
    return 1;
  bloom_init_wo_allocation(bloom, entries, error);
  // allocating space
  bloom->bf = (unsigned char *) calloc(bloom->bytes, sizeof(unsigned char));
  if (bloom->bf == NULL) { // LCOV_EXCL_START
    printf("memory allocation failed, while trying to calloc %lu bytes of "
           "memory!\n",
           bloom->bytes);
    return 1;
  } // LCOV_EXCL_STOP

  bloom->ready = 1;

  return 0;
}

int bloom_check(struct bloom *bloom, const void *buffer, int len) {
  return bloom_check_add(bloom, buffer, len, 0);
}

int bloom_check_ns(struct bloom *bloom, const void *buffer, int len) {
  register size_t a = HASH_FN(buffer, len, bloom->hashSeed);
  register size_t b = HASH_FN(buffer, len, a);
  register size_t x;
  register unsigned int i;
  for (i = 0; i < bloom->hashes; i++) {
    x = (a + i * b) % bloom->bits;
    if (!test_bit(bloom->bf, x)) return 0;
  }
  return 1;
}

int bloom_add(struct bloom *bloom, const void *buffer, int len) {
  return bloom_check_add(bloom, buffer, len, 1);
}

void bloom_add_ns(struct bloom *bloom, const void *buffer, int len) {
  register size_t a = HASH_FN(buffer, len, bloom->hashSeed);
  register size_t b = HASH_FN(buffer, len, a);
  register size_t x;
  register unsigned int i;
  for (i = 0; i < bloom->hashes; i++) {
    x = (a + i * b) % bloom->bits;
    set_bit(bloom->bf, x);
  }
}

void bloom_print(struct bloom *bloom) {
  printf("bloom at %p\n", (void *) bloom);
  printf(" ->entries = %lu\n", bloom->entries);
  printf(" ->error = %f\n", bloom->error);
  printf(" ->bits = %lu\n", bloom->bits);
  printf(" ->bits per elem = %f\n", bloom->bpe);
  printf(" ->bytes = %lu\n", bloom->bytes);
  printf(" ->hash functions = %d\n", bloom->hashes);
#ifdef USE_XXHASH
  const char *hash_fn = "XXHASH";
#elif defined(USE_WYHASH)
  const char *hash_fn = "WYHASH";
#else
  const char *hash_fn = "MURMURHASH";
#endif
  printf(" ->hash function type = %s\n", hash_fn);
}

void bloom_free(struct bloom *bloom) {
  if (bloom->ready) {
#ifdef DEBUG
    printf("Release memory for the byte array\n");
#endif
    free(bloom->bf);
  }
  bloom->bf = NULL;
  bloom->ready = 0;
}

int bloom_reset(struct bloom *bloom) {
  if (!bloom->ready)
    return 1;
  memset(bloom->bf, 0, bloom->bytes);
#ifdef COUNTING_SET_BITS_ON
  bloom.num_set_bits = 0;
#endif
  return 0;
}

const char *bloom_version() { return MAKESTRING(BLOOM_VERSION); }
