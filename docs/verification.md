# Trustworthy verification before parallel scaling

This automation-assisted milestone strengthens the CPU benchmark's correctness checks before measuring OpenMP or GPU performance.

## What changed

- An unchecked run now prints `verification: NOT REQUESTED`; its CSV `verified` value is `not_requested`. Only a requested, successful numerical comparison reports `true` or `PASS`.
- A serial build rejects `--threads` values greater than one, avoiding rows that look like multithread measurements even though the serial binary ignores parallelism.
- Numeric arguments must contain only digits and be positive, in range integers. Inputs such as `17garbage` or `1.5` are rejected instead of silently truncated.
- The comparison rejects different vector lengths, NaN, and infinity. Previously, a NaN difference could slip through because comparison with NaN is false.

## Tests you can run

From the repository root, build a serial executable first:

```sh
mkdir -p bin
c++ -O3 -std=c++20 -Wall -Wextra -Wpedantic src/gemm_bench.cpp -o bin/gemm_bench
c++ -O1 -g -std=c++20 -Wall -Wextra -Wpedantic -fsanitize=address,undefined tests/gemm_correctness.cpp -o bin/gemm_correctness
./bin/gemm_correctness
python3 tests/test_cli_verification.py bin/gemm_bench
python3 -m unittest discover -s scripts -p test_collect_baseline.py
```

The kernel test includes the production implementation in a separate executable, renaming its CLI entry point. This keeps the teaching example small without copying the kernel; a future library split would give a cleaner interface as the project grows. The independent 2x2 expected product tests indexing without relying on the serial reference. Additional cases cover repeated output reuse, odd dimensions, tile sizes larger than the matrix, incorrect values, mismatched lengths, and non-finite results.

## Local validation on November 3, 2026

The serial executable and kernel tests compiled on the available macOS environment. Kernel checks passed under AddressSanitizer and UndefinedBehaviorSanitizer; all five CLI tests and four existing collector tests passed. This is correctness evidence, not a new performance study. The existing October CPU dataset is preserved unchanged and was collected with `--verify`, so the unchecked-output change does not invalidate it.

No OpenMP runtime was available locally, and no OpenMP scaling, CUDA execution, MPI execution, or cluster measurement was performed. The CLI regression suite intentionally targets a serial build. The CI workflow runs these checks on its Ubuntu serial build; a workflow definition is not evidence of a completed remote CI run.

## Learn by changing one thing

1. Calculate the product of `[[1,2],[3,4]]` and `[[5,6],[7,8]]` by hand, then locate that expected result in the kernel test.
2. Trace N=17 with tile=4. Which tile is partial, and why is `min` needed?
3. Run with and without `--verify`. Explain why successful execution alone is not proof of correct arithmetic.
4. In a local copy only, remove the finite-value guard and run the NaN test. Explain why `abs(NaN) > tolerance` does not detect the error.
5. Next milestone: obtain an OpenMP-capable environment, test multiple thread counts for correctness, and only then measure one-thread versus multithread scaling on the same workload.

Remaining limitations: verification still checks the final repetition, uses an absolute tolerance of 1e-9, and has no independent large-matrix BLAS oracle. Integer validation does not impose a memory budget; large valid matrix dimensions can still exhaust memory. These limits should guide later work rather than be interpreted as universal correctness guarantees.
