#!/usr/bin/env bash
set -euo pipefail

binary="${1:-bin/gemm_bench}"
output="${2:-results/local-gemm.csv}"
mkdir -p "$(dirname "$output")"
echo 'size,block_size,repetitions,threads,avg_seconds,gflops,verified' > "$output"

for size in 256 512 1024; do
  "$binary" --size "$size" --block-size 32 --repetitions 3 --threads 1 --verify --csv | tail -n 1 >> "$output"
done

echo "Wrote $output"
