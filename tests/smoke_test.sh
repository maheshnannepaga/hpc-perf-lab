#!/usr/bin/env bash
set -euo pipefail

binary="${1:-bin/gemm_bench}"
output="$($binary --size 64 --block-size 16 --repetitions 1 --verify)"
printf '%s\n' "$output"
grep -q 'verification: PASS' <<<"$output"

