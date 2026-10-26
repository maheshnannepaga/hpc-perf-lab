# First CPU baseline: cache-blocked GEMM

This automated portfolio milestone adds a small, reproducible serial CPU experiment. It was built and executed on a local Darwin arm64 machine with Apple clang 21.0.0 and `-O3 -std=c++20`. Exact CPU model and core count were unavailable in the execution environment; treat these numbers as a local example, not a transferable hardware benchmark.

## Results

One unreported warm-up per configuration, followed by five independent process samples, each averaging three repetitions. All 45 recorded samples requested and passed the existing reference check. Each process uses the same deterministic inputs; samples measure timing variation, not input diversity. N=511 exercises partial tiles.

| Matrix size | Tile | Median time (ms) | Minimum (ms) | Maximum (ms) |
| --- | --- | --- | --- | --- |
| 256 | 16 | 3.371 | 3.078 | 4.053 |
| 256 | 32 | 2.247 | 2.150 | 2.261 |
| 256 | 64 | 2.045 | 2.040 | 2.103 |
| 511 | 16 | 25.211 | 24.895 | 25.219 |
| 511 | 32 | 22.818 | 22.556 | 23.030 |
| 511 | 64 | 20.717 | 20.533 | 20.913 |
| 512 | 16 | 26.699 | 26.361 | 26.867 |
| 512 | 32 | 24.292 | 23.866 | 24.610 |
| 512 | 64 | 19.537 | 19.279 | 19.757 |

Tile 64 had the lowest median among these three tile sizes for each tested matrix size. This observation does not establish an optimal tile size, a speedup over naive GEMM, or general performance on other machines. The configurations ran in fixed order and the system was not isolated or frequency-controlled.

## Reproduce

From the repository root, with Python 3 and a C++20 compiler:

```sh
mkdir -p bin
clang++ -O3 -std=c++20 -Wall -Wextra -Wpedantic src/gemm_bench.cpp -o bin/gemm_bench
python3 -m unittest discover -s scripts -p test_collect_baseline.py
python3 scripts/collect_baseline.py --binary bin/gemm_bench --output results/my-cpu-run --source-revision YOUR_COMMIT_SHA --compiler clang++ --build-command 'clang++ -O3 -std=c++20 -Wall -Wextra -Wpedantic src/gemm_bench.cpp -o bin/gemm_bench'
```

The collector is for a serial build; do not supply an OpenMP build while its metadata says OpenMP is disabled. Use a new output directory each time. If collection fails, the directory may contain metadata only; remove or rename the incomplete directory before retrying. A complete run contains 45 rows in `samples.csv` and `metadata.json`. Raw data: [samples](../results/cpu-baseline-2026-10-26/samples.csv) and [metadata](../results/cpu-baseline-2026-10-26/metadata.json).

## Learn it step by step

1. Read `gemm_blocked` and trace the ii/jj/kk tile loops for N=511 and tile=32. Explain why `min` is needed.
2. Run one verified configuration before running the sweep. Explain what would be misleading about timing a numerically wrong result.
3. Compare the five samples for one configuration. Explain why a single fast sample is weak evidence.
4. Change one tile size locally and predict its effect before measuring. Record what actually happened.
5. Before claiming a speedup, add a timed comparison implementation using the same workload and validation conditions.

## Validation and limitations

The collector has four tests: verification is requested, failed verification is rejected, incorrect workload rows are rejected, and subprocess failures propagate. The C++ source compiled without diagnostics under the stated warning flags. The current numerical check uses absolute tolerance 1e-9 and validates only the final repetition. Timing includes output zeroing; allocation and reference calculation are outside the timed region. The executable rounds time to microseconds. No OpenMP, CUDA, MPI, BLAS comparison, or Slurm run was performed. Generated with automation assistance; the exercises are intended for hands-on learning.
