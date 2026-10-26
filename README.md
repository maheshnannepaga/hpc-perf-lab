# HPC Performance Lab

[![Build and tests](https://github.com/maheshnannepaga/hpc-perf-lab/actions/workflows/ci.yml/badge.svg)](https://github.com/maheshnannepaga/hpc-perf-lab/actions/workflows/ci.yml)

A dense matrix multiplication (GEMM) benchmark suite in C++, built as a hands-on way to learn HPC performance engineering. Starting point: a cache-blocked CPU kernel. End goal: OpenMP, MPI, and CUDA versions of the same problem, benchmarked side by side.

## Why I'm building this

I wanted a project where I couldn't just write parallel code and call it done — I had to prove it was correct and prove it was actually faster. That means deterministic test inputs, a reference check against a naive implementation, and numbers that come from a real scheduler (Slurm), not just my laptop.

## Project status

The current implementation is a **CPU GEMM benchmark in C++20**, with cache blocking and an optional OpenMP parallel loop. Docker and Slurm scripts provide additional ways to run it. **CUDA and MPI implementations are planned.**

A first [local CPU baseline study](docs/cpu-baseline.md) now includes 45 verified samples, metadata, a repeatable collection script, and learning exercises. These are Darwin arm64 serial CPU measurements with incomplete hardware identification, not cluster or GPU results. Speedup comparisons remain pending. The presence of a Slurm script does not establish that cluster measurements have been collected.

## What's here

- C++20 GEMM kernel with cache blocking (configurable tile size)
- Reference implementation + validation so I know the fast version is still correct
- Optional OpenMP build for multi-threaded runs
- Results get written to CSV
- A Slurm script and a Dockerfile so it runs the same way everywhere
- A GitHub Actions job that runs the build + tests on every push

## How it's put together

Input matrices are generated deterministically → run through the blocked GEMM loop → checked against a reference result → timed and converted to GFLOP/s. Cache blocking is intended to improve data reuse. Its performance benefit depends on the hardware, matrix size, tile size, and compiler; it must be established through measurements.

```
CLI args -> deterministic matrices -> blocked GEMM -> validate + GFLOP/s -> CSV
```

## Running it

You need a C++20 compiler and `make`.

```
make
make test
./bin/gemm_bench --size 512 --block-size 32 --repetitions 3 --verify
./scripts/run_benchmarks.sh
```

With OpenMP:

```
make clean
make OPENMP=1
OMP_NUM_THREADS=8 ./bin/gemm_bench --size 1024 --block-size 32 --threads 8 --verify
```

With Docker:

```
docker build -f containers/Dockerfile -t hpc-perf-lab .
docker run --rm hpc-perf-lab
```

## Running it on a cluster

Edit the module line in `slurm/gemm-cpu.slurm` to match whatever's available on your cluster, then:

```
sbatch slurm/gemm-cpu.slurm
```

## How I'm measuring performance

`GFLOP/s = 2 * N^3 / elapsed_seconds / 1e9`

For any number I report, I log the CPU, core count, OS, compiler, flags, matrix/tile size, thread count, repetitions, and whether validation passed. If two runs aren't using the same workload and both haven't passed validation, I don't compare them.

Full writeup: [docs/experiment-plan.md](docs/experiment-plan.md)


## Measurement boundaries

- The benchmark reports the mean runtime across repetitions. The timed region includes clearing the output matrix and executing the blocked kernel.
- `--verify` compares the final result against a serial reference using an absolute tolerance of `1e-9`. Use this flag for any published measurement; without it, the current program's success/verification output does not establish numerical validation.
- The serial reference is used for correctness, not timed as a performance baseline by the current executable.
- Thread counts affect execution only in an OpenMP-enabled build. Record the build configuration alongside results.
- No comparison with optimized BLAS or GPU libraries is claimed.

## GPU and HPC roadmap

1. Publish CPU measurements across matrix sizes and tile sizes, with hardware and compiler metadata.
2. Measure OpenMP scaling against a one-thread run of the same kernel, with correctness checks enabled.
3. Add a CUDA baseline and a shared-memory tiled GEMM kernel, including non-tile-aligned matrix sizes.
4. Measure GPU kernel time separately from allocation and host/device transfers; document precision and validation tolerances.
5. Compare equivalent workloads with cuBLAS and document the remaining performance gap.
6. Explore MPI decomposition and communication overhead after the single-node measurements are established.

These are future milestones, not completed features. The project focuses on correct, measurable, reproducible performance engineering and documented tradeoffs.

## License

MIT — see [LICENSE](LICENSE).
