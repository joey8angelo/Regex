#!/usr/bin/env bash

OUTPUT_FILE="results/grep_speed.txt"

test_speed() {
  local test_pattern
  local test_string

  test_pattern=$(printf 'a?%.0s' $(seq 1 "$1"))$(printf 'a%.0s' $(seq 1 "$1"))
  test_string=$(printf 'a%.0s' $(seq 1 "$1"))

  tmpfile=$(mktemp)
  echo "$test_string" > "$tmpfile"

  start_time=$(date +%s.%N)
  grep -Eq "$test_pattern" "$tmpfile"
  end_time=$(date +%s.%N)

  duration=$(awk -v start="$start_time" -v end="$end_time" 'BEGIN {print (end - start) * 1000000}')

  rm -f "$tmpfile"

  echo "$duration"
}

for ((i=1; i<=$1; i++)); do
  total_time=0
  for ((j=1; j<=$2; j++)); do
    duration=$(test_speed "$i")
    total_time=$(awk -v total="$total_time" -v duration="$duration" 'BEGIN {print total + duration}')
  done
  average_time=$(awk -v total="$total_time" -v rounds="$2" 'BEGIN {printf "%.6f", total/rounds}')
  echo Grep "$i" "$average_time"
  echo "$average_time" >> "$OUTPUT_FILE"
done
