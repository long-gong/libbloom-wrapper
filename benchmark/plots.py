#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import os.path

plt.style.use("seaborn-bright")
matplotlib.rcParams.update({"font.size": 24})

LIB_NAMES = ["libbf", "libbloom-org", "libbloom", "cppbloom"]
SHORT_LIB_NAMES = {
    "libbf": "libbf",
    "libbloom-org": "libbloom",
    "cppbloom": "cppbloom",
    "libbloom": "libbloom-x",
}
MYDPI = 100
BAR_WIDTH = 0.2
GEN_MD = True


def fpr_str2num(fpr_str):
    return float(fpr_str[:-1]) / 100.0


def simplify_fpr_str(fpr_str):
    import re

    last_ch = re.findall(r"[^0.%]", fpr_str)[-1]
    sep1 = fpr_str.rfind(last_ch) + 1
    sep2 = fpr_str.find(".")
    sep = sep1 if sep1 > sep2 else sep2
    return fpr_str[:sep] + "%"


def get_bench_keytype(bench_result_filename):
    name, _ = os.path.splitext(bench_result_filename)
    return name.split("_")[-1]


def fpr_plots(csvfile, filter_key, filter_value):
    FIGSIZE = (15.63, 9.50)
    figid = int(float(filter_value[:-1]) * 100)
    df = pd.read_csv(csvfile)
    ddf = df[df[filter_key] == filter_value]
    xt = set(ddf["# of items (million)"].tolist())
    N = len(xt)
    ind = np.arange(N)
    width = BAR_WIDTH
    plt.figure(figsize=FIGSIZE, dpi=MYDPI)
    for i, lib in enumerate(LIB_NAMES):
        dddf = ddf[df["library"] == lib]
        plt.bar(
            ind + width * i,
            [fpr_str2num(fpr_str) for fpr_str in dddf["false positive rate"].tolist()],
            width,
            label=SHORT_LIB_NAMES[lib],
        )
    plt.ylabel("False Positive Rate")
    plt.xlabel("Number of Inserted Items (million)")
    plt.xticks(ind + width * (len(LIB_NAMES) - 1) / 2.0, ("1", "2", "5", "10", "20", "50", "100"))
    plt.legend(
        bbox_to_anchor=(0.0, 1.02, 1.0, 0.102),
        loc="lower left",
        ncol=len(LIB_NAMES),
        mode="expand",
        borderaxespad=0.0,
    )
    plot_dir = os.path.join("plots", get_bench_keytype(csvfile))
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)
    plt.savefig(f"{plot_dir}/fpr_{figid}.svg", format="svg", dpi=100)
    if GEN_MD:
        return "+ desired fpr = %s\n\t![false positive rate %i](%s)" % (
            simplify_fpr_str(filter_value),
            figid,
            f"{plot_dir}/fpr_{figid}.svg",
        )


def constr_speed_plots(csvfile, filter_key, filter_value):
    FIGSIZE = (15.63, 9.00)
    figid = int(filter_value)
    df = pd.read_csv(csvfile)
    ddf = df[df[filter_key] == filter_value]
    xt = set(ddf["desired fpr"].tolist())
    N = len(xt)
    ind = np.arange(N)
    width = BAR_WIDTH
    plt.figure(figsize=FIGSIZE, dpi=MYDPI)
    for i, lib in enumerate(LIB_NAMES):
        dddf = ddf[df["library"] == lib]
        plt.bar(
            ind + width * i,
            dddf["construction speed (million keys/sec)"].tolist(),
            width,
            label=SHORT_LIB_NAMES[lib],
        )
    plt.ylabel("Construction Speed (million keys/sec)")
    plt.xlabel("Desired False Positive Rate")
    plt.xticks(ind + width * (len(LIB_NAMES) - 1) / 2.0, ("$10^{-1}$", "$10^{-2}$", "$10^{-3}$", "$10^{-4}$"))
    plt.legend(
        bbox_to_anchor=(0.0, 1.02, 1.0, 0.102),
        loc="lower left",
        ncol=len(LIB_NAMES),
        mode="expand",
        borderaxespad=0.0,
    )
    plot_dir = os.path.join("plots", get_bench_keytype(csvfile))
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)

    plt.savefig(f"{plot_dir}/constr_speed_{figid}.svg", format="svg", dpi=100)
    if GEN_MD:
        return (
            "+ %.0f millions (items inserted during construction)\n\t![construction speed %i](%s)"
            % (filter_value, figid, f"{plot_dir}/constr_speed_{figid}.svg")
        )


def check_speed_plots(csvfile, filter_key, filter_value):
    FIGSIZE = (15.63, 9.00)
    figid = int(filter_value)
    df = pd.read_csv(csvfile)
    ddf = df[df[filter_key] == filter_value]
    xt = set(ddf["desired fpr"].tolist())
    N = len(xt)
    ind = np.arange(N)
    width = BAR_WIDTH
    plt.figure(figsize=FIGSIZE, dpi=MYDPI)
    for i, lib in enumerate(LIB_NAMES):
        dddf = ddf[df["library"] == lib]
        plt.bar(
            ind + width * i,
            dddf["check speed (million keys/sec)"].tolist(),
            width,
            label=SHORT_LIB_NAMES[lib],
        )
    plt.ylabel("Check Speed (million keys/sec)")
    plt.xlabel("Desired False Positive Rate")
    plt.xticks(ind + width * (len(LIB_NAMES) - 1) / 2.0, ("$10^{-1}$", "$10^{-2}$", "$10^{-3}$", "$10^{-4}$"))
    plt.legend(
        bbox_to_anchor=(0.0, 1.02, 1.0, 0.102),
        loc="lower left",
        ncol=len(LIB_NAMES),
        mode="expand",
        borderaxespad=0.0,
    )
    plot_dir = os.path.join("plots", get_bench_keytype(csvfile))
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)

    plt.savefig(f"{plot_dir}/check_speed_{figid}.svg", format="svg", dpi=100)
    if GEN_MD:
        return (
            "+ %.0f millions (items inserted during construction)\n\t![check speed %i](%s)"
            % (filter_value, figid, f"{plot_dir}/check_speed_{figid}.svg")
        )


def space_plots(csvfile, filter_key, filter_value):
    FIGSIZE = (15.63, 9.00)
    figid = int(filter_value)
    df = pd.read_csv(csvfile)
    ddf = df[df[filter_key] == filter_value]
    xt = set(ddf["desired fpr"].tolist())
    N = len(xt)
    ind = np.arange(N)
    width = BAR_WIDTH
    plt.figure(figsize=FIGSIZE, dpi=MYDPI)
    for i, lib in enumerate(LIB_NAMES):
        dddf = ddf[df["library"] == lib]
        plt.bar(
            ind + width * i,
            dddf["space (bits per item)"].tolist(),
            width,
            label=SHORT_LIB_NAMES[lib],
        )
    plt.ylabel("Space (bits per item)")
    plt.xlabel("Desired False Positive Rate")
    plt.xticks(ind + width * (len(LIB_NAMES) - 1) / 2.0, ("$10^{-1}$", "$10^{-2}$", "$10^{-3}$", "$10^{-4}$"))
    plt.legend(
        bbox_to_anchor=(0.0, 1.02, 1.0, 0.102),
        loc="lower left",
        ncol=len(LIB_NAMES),
        mode="expand",
        borderaxespad=0.0,
    )
    plot_dir = os.path.join("plots", get_bench_keytype(csvfile))
    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)

    plt.savefig(f"{plot_dir}/space_{figid}.svg", format="svg", dpi=100)
    if GEN_MD:
        return (
            "+ %.0f millions (items inserted during construction)\n\t![space %i](%s)"
            % (filter_value, figid, f"space_{figid}.svg")
        )


if __name__ == "__main__":
    import warnings

    warnings.simplefilter("ignore")
    res_filenames = [
        "./raw_results/benchmark_results_64u.csv",
        "./raw_results/benchmark_results_32u.csv",
    ]
    metrics = ["false positive rate", "construction speed", "check speed", "space"]

    for res_filename in res_filenames:
        MD = dict(zip(metrics, [[] for _ in metrics]))
        df = pd.read_csv(res_filename)
        diff_dfprs = set(df["desired fpr"].tolist())
        for dfpr in diff_dfprs:
            MD["false positive rate"].append(
                (float(dfpr[:-1]), fpr_plots(res_filename, "desired fpr", dfpr))
            )
        diff_inserted = set(df["# of items (million)"].tolist())
        for inserted in diff_inserted:
            MD["construction speed"].append(
                (
                    inserted,
                    constr_speed_plots(res_filename, "# of items (million)", inserted),
                )
            )
            MD["check speed"].append(
                (
                    inserted,
                    check_speed_plots(res_filename, "# of items (million)", inserted),
                )
            )
            MD["space"].append(
                (inserted, space_plots(res_filename, "# of items (million)", inserted))
            )
        if GEN_MD:
            print("\n\n\n")
            for key, val in MD.items():
                print(f"## {key}\n\n")
                print("\n".join([x[1] for x in sorted(val, key=lambda x: x[0])]))
                print("\n\n")

