#!/usr/bin/env python3

import re
import sys
import timeit


def test_speed(n):
    test_pattern = r"a?" * n + r"a" * n
    test_string = "a" * n

    pattern = re.compile(test_pattern)

    start_time = timeit.default_timer()
    _ = pattern.search(test_string)
    end_time = timeit.default_timer()

    return (end_time - start_time) * 1_000_000


if __name__ == "__main__":
    num_tests = int(sys.argv[1])
    num_rounds = int(sys.argv[2])
    for i in range(1, num_tests + 1):
        total_time = 0
        for _ in range(num_rounds):
            total_time += test_speed(i)
        average_time = total_time / num_rounds
        with open("results/python_speed.txt", "a") as f:
            f.write(f"{average_time:.6f}\n")
        print(f"Python {i}: {average_time:.6f}")
