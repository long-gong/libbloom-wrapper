# Benchmark Results

Here, we only present the results for using 64-bit unsigned integers as keys, the results for using 32-bit unsigned integers as keys lead to the same conclusion, which can be find in [32u.md](./32u.md).

## Benchmarked

+ [cppbloom](https://github.com/ArashPartow/bloom): C++ Bloom Filter Library 
+ [libbloom](https://github.com/jvirkki/libbloom): A simple and small bloom filter implementation in plain C
+ [libbloom-x](https://github.com/long-gong/libbloom-x): C++/Python wrapper for libbloom (with some modificatons)
+ [libbf](https://github.com/mavam/libbf): Bloom filters for C++11

All algorithms are compiled with `gcc/g++ 7.5.0` with `-O3` on an Intel(R) Core(TM) i7-7700 CPU @ 3.60GHz running Ubuntu 18.04.

## Overall Preferences

cppbloom > libbloom (libbloom-x) > libbf

## False Positive Rate

+ desired fpr = 0.01%
        ![false positive rate 1](plots/64u/fpr_1.svg)
+ desired fpr = 0.1%
        ![false positive rate 10](plots/64u/fpr_10.svg)
+ desired fpr = 1%
        ![false positive rate 100](plots/64u/fpr_100.svg)
+ desired fpr = 10%
        ![false positive rate 1000](plots/64u/fpr_1000.svg)

## Construction Speed

+ 1 millions (items inserted during construction)
        ![construction speed 1](plots/64u/constr_speed_1.svg)
+ 2 millions (items inserted during construction)
        ![construction speed 2](plots/64u/constr_speed_2.svg)
+ 5 millions (items inserted during construction)
        ![construction speed 5](plots/64u/constr_speed_5.svg)
+ 10 millions (items inserted during construction)
        ![construction speed 10](plots/64u/constr_speed_10.svg)
+ 20 millions (items inserted during construction)
        ![construction speed 20](plots/64u/constr_speed_20.svg)
+ 50 millions (items inserted during construction)
        ![construction speed 50](plots/64u/constr_speed_50.svg)
+ 100 millions (items inserted during construction)
        ![construction speed 100](plots/64u/constr_speed_100.svg)

## Check Speed

+ 1 millions (items inserted during construction)
        ![check speed 1](plots/64u/check_speed_1.svg)
+ 2 millions (items inserted during construction)
        ![check speed 2](plots/64u/check_speed_2.svg)
+ 5 millions (items inserted during construction)
        ![check speed 5](plots/64u/check_speed_5.svg)
+ 10 millions (items inserted during construction)
        ![check speed 10](plots/64u/check_speed_10.svg)
+ 20 millions (items inserted during construction)
        ![check speed 20](plots/64u/check_speed_20.svg)
+ 50 millions (items inserted during construction)
        ![check speed 50](plots/64u/check_speed_50.svg)
+ 100 millions (items inserted during construction)
        ![check speed 100](plots/64u/check_speed_100.svg)

## Space

+ 1 millions (items inserted during construction)
        ![space 1](plots/64u/space_1.svg)
+ 2 millions (items inserted during construction)
        ![space 2](plots/64u/space_2.svg)
+ 5 millions (items inserted during construction)
        ![space 5](plots/64u/space_5.svg)
+ 10 millions (items inserted during construction)
        ![space 10](plots/64u/space_10.svg)
+ 20 millions (items inserted during construction)
        ![space 20](plots/64u/space_20.svg)
+ 50 millions (items inserted during construction)
        ![space 50](plots/64u/space_50.svg)
+ 100 millions (items inserted during construction)
        ![space 100](plots/64u/space_100.svg)
