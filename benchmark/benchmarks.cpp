// stolen from
// https://github.com/efficient/cuckoofilter/blob/master/benchmarks/conext-table3.cc

#include <climits>
#include <iomanip>
#include <type_traits>
#include <vector>

#include "BloomFilter.h"
#include "bf/all.hpp"
#include "bloom_filter.hpp"
#include "random.h"
#include "timing.h"

using namespace std;
// The number of items sampled when determining the false positive rate
const size_t ONE_MILLION = 1000 * 1000;
const size_t FPR_SAMPLE_SIZE = ONE_MILLION;
const char *RESULT_HEADER =
    "library,# of items (million),inserted item type,desired fpr,false "
    "positive rate,construction speed (million keys/sec),check speed "
    "(million keys/sec),space (bits per item)";
const char *RESULT_FMT = "%s,%.4f,%s,%.8f%%,%.8f%%,%.8f,%.8f,%.8f\n";

struct Metrics {
  double add_count; // # of items (million)
  double space;     // bits per item
  double fpr;       // false positive rate (%)
  double speed;     // const. speed (million keys/sec)
  double check_speed;
};

template <typename T> vector<T> gen_random(size_t count) {
  if constexpr (is_same<T, uint32_t>::value)
    return GenerateRandom32(count);
  else if constexpr (is_same<T, uint64_t>::value)
    return GenerateRandom64(count);
  else
    throw std::runtime_error("Not Implemented!");
}

template <typename BF> BF create_bf(size_t add_count, double error) {
  if constexpr (is_same<BF, bf::basic_bloom_filter>::value)
    return BF(error, add_count);
  else if constexpr (is_same<BF, bloom_filter>::value) {
    bloom_parameters params;
    params.projected_element_count = add_count;
    params.false_positive_probability = error;
    params.compute_optimal_parameters();
    return BF(params);
  } else if constexpr (is_same<BF, BloomFilter>::value)
    return BF(add_count, error);
  else
    throw std::runtime_error("Not Supported!");
}
template <typename BF, typename T>
Metrics BloomFilterBenchmark(size_t add_count, double error) {
  vector<T> input = gen_random<T>(size_t(1.2 * add_count) + FPR_SAMPLE_SIZE);

  auto check_end = add_count + FPR_SAMPLE_SIZE;
  assert(input.size() >= check_end);
  auto f = create_bf<BF>(add_count, error);

  uint64_t start_time, constr_time, check_time;

  start_time = NowNanos();
  for (size_t i = 0; i < add_count; ++i)
    if constexpr (is_same<BF, bloom_filter>::value)
      f.insert(input[i]);
    else
      f.add(input[i]);
  constr_time = NowNanos() - start_time;

  // Count false positives:
  size_t false_positive_count = 0;
  size_t absent = FPR_SAMPLE_SIZE;

  start_time = NowNanos();
  for (size_t i = add_count; i < check_end; ++i)
    if constexpr (std::is_same<BF, bf::basic_bloom_filter>::value)
      false_positive_count += (f.lookup(input[i]) ? 1 : 0);
    else
      false_positive_count += (f.contains(input[i]) ? 1 : 0);
  check_time = NowNanos() - start_time;

  // Calculate metrics:
  const auto time = constr_time / static_cast<double>(1000 * 1000 * 1000);
  const auto ch_time = check_time / static_cast<double>(1000 * 1000 * 1000);
  Metrics result;
  result.add_count = static_cast<double>(add_count) / (1000 * 1000);
  if constexpr (std::is_same<BF, bf::basic_bloom_filter>::value)
    result.space =
        static_cast<double>(bf::basic_bloom_filter::m(error, add_count)) /
        add_count;
  else
    result.space = static_cast<double>(f.size()) / add_count;
  result.fpr = (100.0 * false_positive_count) / absent;
  result.speed = (add_count / time) / (1000 * 1000);
  result.check_speed = (FPR_SAMPLE_SIZE / ch_time) / (1000 * 1000);
  return result;
}

template <typename BF> const char *get_libname() {
  if (std::is_same<BF, BloomFilter>::value)
    return "libbloom";
  else if (std::is_same<BF, bloom_filter>::value)
    return "cppbloom";
  else if (std::is_same<BF, bf::bloom_filter>::value)
    return "libbf";
  else
    return "NotSupported";
}

template <typename T> const char *get_typename() {
  if (std::is_same<T, uint32_t>::value)
    return "uint32_t";
  else if (std::is_same<T, uint64_t>::value)
    return "uin64_t";
  else
    return "NotSupported";
}
template <typename T>
void BenchmarkAllInOne(size_t add_count, double fpr, FILE *fp) {
  {
    const auto res = BloomFilterBenchmark<BloomFilter, T>(add_count, fpr);
    fprintf(fp, RESULT_FMT, get_libname<BloomFilter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
    fprintf(stdout, RESULT_FMT, get_libname<BloomFilter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
  }
  {
    const auto res = BloomFilterBenchmark<bloom_filter, T>(add_count, fpr);
    fprintf(fp, RESULT_FMT, get_libname<bloom_filter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
    fprintf(stdout, RESULT_FMT, get_libname<bloom_filter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
  }
  {
    const auto res =
        BloomFilterBenchmark<bf::basic_bloom_filter, T>(add_count, fpr);
    fprintf(fp, RESULT_FMT, get_libname<bf::bloom_filter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
    fprintf(stdout, RESULT_FMT, get_libname<bf::bloom_filter>(), res.add_count,
            get_typename<T>(), fpr * 100, res.fpr, res.speed, res.check_speed,
            res.space);
  }
}

int main(int argc, char **argv) {

  const std::vector<size_t> TEST_ITEMS_FACTOR({1, 2, 5, 10, 20, 50, 100});
  const std::vector<double> TEST_ERROR({0.1, 0.01, 0.001, 0.0001});

  const char *filename32 = "benchmark_results_32u.csv";
  FILE *fp32 = fopen(filename32, "w");
  if (fp32 == NULL) {
    fprintf(stderr, "Failed to create file %s\n", filename32);
    exit(1);
  }

  fprintf(fp32, "%s\n", RESULT_HEADER);

  const char *filename64 = "benchmark_results_64u.csv";
  FILE *fp64 = fopen(filename64, "w");
  if (fp64 == NULL) {
    fprintf(stderr, "Failed to create file %s\n", filename64);
    exit(1);
  }
  fprintf(fp64, "%s\n", RESULT_HEADER);

  fprintf(stdout, "%s\n", RESULT_HEADER);
  for (size_t fac : TEST_ITEMS_FACTOR) {
    size_t add_count = ONE_MILLION * fac;
    for (auto fpr : TEST_ERROR) {
      BenchmarkAllInOne<uint32_t>(add_count, fpr, fp32);
      BenchmarkAllInOne<uint64_t>(add_count, fpr, fp64);
    }
  }

  fclose(fp32);
  fclose(fp64);
}
