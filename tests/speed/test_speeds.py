#!/usr/bin/env python3

import os
import subprocess

import matplotlib.pyplot as plt

NUM_TESTS = 100
NUM_ROUNDS = 3
TIMEOUT = 5


def run_timeout(cmd, timeout=TIMEOUT):
    try:
        subprocess.run(cmd, timeout=timeout, check=True)
    except subprocess.TimeoutExpired:
        print(f"Command {cmd} timed out after {timeout} seconds.")
    except subprocess.CalledProcessError as e:
        print(f"Command {cmd} failed with error: {e}")


if __name__ == "__main__":
    plt.figure(figsize=(10, 6))
    # test Regex speed
    if not os.path.exists("results/cpp_Regex_speed.txt"):
        run_timeout(["./build/cpp_Regex_speed", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/cpp_Regex_speed.txt", "r") as f:
        cpp_regex_data = f.readlines()
        plt.plot(
            list(range(1, len(cpp_regex_data) + 1)),
            [float(x.strip()) for x in cpp_regex_data],
            label="C++ Regex",
        )

    # test python speed
    if not os.path.exists("results/python_speed.txt"):
        run_timeout(["./python_speed.py", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/python_speed.txt", "r") as f:
        python_data = f.readlines()
        plt.plot(
            list(range(1, len(python_data) + 1)),
            [float(x.strip()) for x in python_data],
            label="Python",
        )

    # test perl speed
    if not os.path.exists("results/perl_speed.txt"):
        run_timeout(["./perl_speed.sh", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/perl_speed.txt", "r") as f:
        perl_data = f.readlines()
        plt.plot(
            list(range(1, len(perl_data) + 1)),
            [float(x.strip()) for x in perl_data],
            label="Perl",
        )

    # test awk speed
    if not os.path.exists("results/awk_speed.txt"):
        run_timeout(["./awk_speed.sh", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/awk_speed.txt", "r") as f:
        awk_data = f.readlines()
        plt.plot(
            list(range(1, len(awk_data) + 1)),
            [float(x.strip()) for x in awk_data],
            label="Awk",
        )

    # test grep speed
    if not os.path.exists("results/grep_speed.txt"):
        run_timeout(["./grep_speed.sh", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/grep_speed.txt", "r") as f:
        grep_data = f.readlines()
        plt.plot(
            list(range(1, len(grep_data) + 1)),
            [float(x.strip()) for x in grep_data],
            label="Grep",
        )

    # test cpp std::regex speed
    if not os.path.exists("results/cpp_std_speed.txt"):
        run_timeout(["./cpp_std_speed", str(NUM_TESTS), str(NUM_ROUNDS)])
    with open("results/cpp_std_speed.txt", "r") as f:
        cpp_std_data = f.readlines()
        plt.plot(
            list(range(1, len(cpp_std_data) + 1)),
            [float(x.strip()) for x in cpp_std_data],
            label="C++ std::regex",
        )

    # data from the previous implementation
    if os.path.exists("results/cpp_old_speed.txt"):
        with open("results/cpp_old_speed.txt", "r") as f:
            cpp_old_data = f.readlines()
            plt.plot(
                list(range(1, len(cpp_old_data) + 1)),
                [float(x.strip()) for x in cpp_old_data],
                label="C++ Old Regex",
            )

    plt.yscale("log")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)

    plt.xlabel("Match a^n against (a?)^na^n")
    plt.ylabel("Time (microseconds)")
    plt.legend()

    plt.show()
