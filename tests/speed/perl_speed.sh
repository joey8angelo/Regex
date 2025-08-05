#!/usr/bin/env bash

OUTPUT_FILE="results/perl_speed.txt"

test_speed() {
  local test_pattern
  local test_string

  test_pattern=$(printf 'a?%.0s' $(seq 1 "$1"))$(printf 'a%.0s' $(seq 1 "$1"))
  test_string=$(printf 'a%.0s' $(seq 1 "$1"))

  start_time=$(date +%s.%N)
  perl -e '
    my $str = shift;
    my $pat = shift;
    exit($str =~ /^$pat$/ ? 0 : 1);
  ' "$test_string" "$test_pattern" >/dev/null 2>&1
  end_time=$(date +%s.%N)

  duration=$(echo "($end_time - $start_time) * 1000000" | bc)

  echo "$duration"
}

for ((i=1; i<=$1; i++)); do
  total_time=0
  for ((j=1; j<=$2; j++)); do
    duration=$(test_speed "$i")
    total_time=$(awk -v total="$total_time" -v duration="$duration" 'BEGIN {print total + duration}')
  done
  average_time=$(awk -v total="$total_time" -v rounds="$2" 'BEGIN {printf "%.6f", total/rounds}')
  echo Perl "$i" "$average_time"
  echo "$average_time" >> "$OUTPUT_FILE"
done
