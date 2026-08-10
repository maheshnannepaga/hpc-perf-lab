# HPC Performance Lab

A dense matrix multiplication (GEMM) benchmark suite in C++, built as a hands-on way to learn HPC performance engineering. Starting point: a cache-blocked CPU kernel. End goal: OpenMP, MPI, and CUDA versions of the same problem, benchmarked side by side.

## Why I'm building this

I wanted a project where I couldn't just write parallel code and call it done — I had to prove it was correct and prove it was actually faster. That means deterministic test inputs, a reference check against a naive implementation, and numbers that come from a real scheduler (Slurm), not just my laptop.

Right now this is Week 1: a working blocked GEMM kernel in C++20, baseline timings, and three ways to run it (locally, in a container, or via Slurm).

## What's here

- C++20 GEMM kernel with cache blocking (configurable tile size)
- Reference implementation + validation so I know the fast version is still correct
- Optional OpenMP build for multi-threaded runs
- Results get written to CSV
- A Slurm script and a Dockerfile so it runs the same way everywhere
- A GitHub Actions job that runs the build + tests on every push

## How it's put together

Input matrices are generated deterministically → run through the blocked GEMM loop → checked against a reference result → timed and converted to GFLOP/s. The blocking keeps each output tile in cache during the inner loop, which is what makes this faster than the naive triple loop.

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


## Note

This isn't meant to compete with BLAS — it's a learning project focused on writing correct, measurable, reproducible parallel code and documenting the tradeoffs along the way.

## License

MIT — see [LICENSE](LICENSE).
