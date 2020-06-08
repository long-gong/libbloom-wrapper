#include <gtest/gtest.h>
#include <BloomFilter.h>
#include <cmath>

TEST(BloomFilterTest, ConsturctorArgumentsShouldBeValid) {
  EXPECT_NO_THROW(BloomFilter(1000, 0.2));
  EXPECT_THROW(BloomFilter(0, 1.0), std::runtime_error);
  EXPECT_THROW(BloomFilter(10, 0), std::runtime_error);
  EXPECT_THROW(BloomFilter(0, 0), std::runtime_error);
}

TEST(BloomFilterTest, ValidConsturctorArgumentsShouldLeadToOptimalConfig) {
  size_t items = 10000;
  double error = 0.01;
  unsigned seed = 9021;
  auto bf = BloomFilter(items, error, seed);
  EXPECT_EQ(seed, bf.hash_seed());
  auto optimal_hashes = (size_t)std::ceil(-std::log2(error));
  EXPECT_EQ(optimal_hashes, bf.num_hashes());
  auto optimal_size = (size_t)std::ceil(  -std::log(error) * items / std::log(2) / std::log(2));
  EXPECT_EQ(optimal_size, bf.size());
}

TEST(BloomFilterTest, CopyConsturctor) {
  size_t items = 10000;
  double error = 0.01;
  unsigned seed = 9021;
  auto bf_ = BloomFilter(items, error, seed);
  auto bf = BloomFilter(bf_);
  EXPECT_EQ(seed, bf.hash_seed());
  auto optimal_hashes = (size_t)std::ceil(-std::log2(error));
  EXPECT_EQ(optimal_hashes, bf.num_hashes());
  auto optimal_size = (size_t)std::ceil(  -std::log(error) * items / std::log(2) / std::log(2));
  EXPECT_EQ(optimal_size, bf.size());
}

TEST(BloomFilterTest, CopyAssignment) {
  size_t items = 10000;
  double error = 0.01;
  unsigned seed = 9021;
  auto bf_ = BloomFilter(items, error, seed);
  auto bf = BloomFilter(4*items, error);
  bf = bf_;
  EXPECT_EQ(seed, bf.hash_seed());
  auto optimal_hashes = (size_t)std::ceil(-std::log2(error));
  EXPECT_EQ(optimal_hashes, bf.num_hashes());
  auto optimal_size = (size_t)std::ceil(  -std::log(error) * items / std::log(2) / std::log(2));
  EXPECT_EQ(optimal_size, bf.size());
}

TEST(BloomFilter, ConstructFromExistingBitMap) {
  size_t items = 1000;
  double error = 0.1;
  auto optimal_size = (size_t)std::ceil(  -std::log(error) * items / std::log(2) / std::log(2));
  size_t offset = (optimal_size % 8 == 0)? 0 : 1;
  auto optimal_byte_size = optimal_size / 8 + offset;
  std::vector<unsigned char> raw_bf_cp(optimal_byte_size, 1u);
  EXPECT_THROW(BloomFilter(items * 4, error, &raw_bf_cp[0], optimal_byte_size), std::runtime_error);
  auto* raw_bf = (unsigned  char*)calloc(optimal_byte_size, sizeof(unsigned char));
  std::fill(raw_bf, raw_bf + optimal_byte_size, 1u);
  auto bf = BloomFilter(items, error, raw_bf, optimal_byte_size);
  EXPECT_EQ(optimal_byte_size, bf.popcount());
  EXPECT_TRUE(raw_bf == nullptr);
}

TEST(BloomFilter, EmptyBloomFilter) {
  size_t items = 10000;
  double error = 0.01;
  unsigned seed = 9021;
  auto bf = BloomFilter(items, error, seed);

  for (int i = 0;i < 100;++ i) {
    EXPECT_FALSE(bf.contains(i));
  }

  EXPECT_EQ(0lu, bf.popcount());
}

TEST(BloomFilterTest, AddAndCheck) {
  size_t items = 10000;
  double error = 0.1;
  unsigned seed = 9021;
  auto bf = BloomFilter(items, error, seed);

  // int
  for (int i = 0;i < 100;++ i) bf.add(i);
  for (int i = 0;i < 100;++ i) EXPECT_TRUE(bf.contains(i));

  bf.reset(); // reset all bits to zero
  EXPECT_EQ(0lu, bf.popcount());

  // long
  for (long i = 0;i < 100;++ i) bf.add(i);
  for (long i = 0;i < 100;++ i) EXPECT_TRUE(bf.contains(i));

  bf.reset(); // reset all bits to zero
  EXPECT_EQ(0lu, bf.popcount());

  // std::string
  for (int i = 0;i < 100; ++ i) bf.add(std::to_string(i));
  for (int i = 0;i < 100;++ i) EXPECT_TRUE(bf.contains(std::to_string(i)));
}

TEST(BloomFilter, FalsePositiveProbability) {
  size_t items = 100000;
  double error = 0.1;
  unsigned seed = 9021;
  auto bf = BloomFilter(items, error, seed);

  for (int i = 0;i < (int)items;++ i) bf.add(i);
  size_t cf = 0, ct = 0;
  for (int i = (int)items;i < 2 * (int)items;++ i) {
    if (bf.contains(i)) cf ++;
    ct ++;
  }
  printf("Expected false positive rate: %.8f, observed false positive rate: %.8f\n",bf.effective_fpp(), (double)cf / ct);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
