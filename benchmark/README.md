# Benchmark Results

Here, we only present the results for using 64-bit unsigned integers as keys, the results for using 32-bit unsigned integers as keys can be find in [32u.md](./32u.md)

## false positive rate


+ desired fpr = 0.01%
        ![false positive rate 1](plots/64u/fpr_1.svg)
+ desired fpr = 0.1%
        ![false positive rate 10](plots/64u/fpr_10.svg)
+ desired fpr = 1%
        ![false positive rate 100](plots/64u/fpr_100.svg)
+ desired fpr = 10%
        ![false positive rate 1000](plots/64u/fpr_1000.svg)



## construction speed


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



## check speed


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



## space


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