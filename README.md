# HPC Performance Lab

A reproducible C++ performance-engineering lab for dense matrix multiplication. It begins with a cache-blocked CPU kernel and evolves into an OpenMP, MPI, and CUDA benchmark suite.

## Why this project

HPC and AI-infrastructure work requires repeatable experiments, numerical validation, scheduler-aware execution, and performance evidence—not only parallel code. This repository demonstrates that workflow in a compact codebase.

**Week 1 scope:** a correct, cache-blocked C++ GEMM kernel; baseline measurements; local, container, and Slurm execution.

## Capabilities

- Deterministic input generation and reference-result validation
- Cache-blocked C++20 GEMM with configurable matrix and tile sizes
- Optional OpenMP build for shared-memory scaling
- CSV benchmark output
- Slurm batch script, container definition, and GitHub Actions smoke test

## Architecture

```text
CLI options -> deterministic matrices -> blocked GEMM -> validation + GFLOP/s
                                              |
                                 local script / Slurm / container
                                              |
                                    reproducible CSV record
```

The row-major blocked loop order keeps each output tile private to one OpenMP iteration and reuses cache-resident blocks. It is deliberately small enough to profile and improve in later milestones.

## Quick start

Requirements: a C++20 compiler and `make`.

```bash
make
make test
./bin/gemm_bench --size 512 --block-size 32 --repetitions 3 --verify
./scripts/run_benchmarks.sh
```

For OpenMP on Linux with a compatible compiler:

```bash
make clean
make OPENMP=1
OMP_NUM_THREADS=8 ./bin/gemm_bench --size 1024 --block-size 32 --threads 8 --verify
```

Container run:

```bash
docker build -f containers/Dockerfile -t hpc-perf-lab .
docker run --rm hpc-perf-lab
```

## Slurm

Update the module line in `slurm/gemm-cpu.slurm` for your cluster, then run:

```bash
sbatch slurm/gemm-cpu.slurm
```

## Measuring performance

The benchmark reports average runtime and throughput:

`GFLOP/s = 2 x N^3 / elapsed_seconds / 10^9`

For every published result, record the CPU model, core count, OS, compiler and flags, matrix and tile size, thread count, repetitions, and verification result. Never claim a speedup unless both measurements use the same workload and have passed validation.

See [the experiment plan](docs/experiment-plan.md).

## Roadmap

- [x] Week 1: blocked GEMM, validation, CSV, Slurm, and container workflow
- [ ] Week 2: OpenMP scaling, CPU affinity notes, `perf` profile, and charts
- [ ] Week 3: MPI distributed GEMM and communication measurements
- [ ] Week 4: CUDA kernel, Nsight profile, CPU-versus-GPU report
- [ ] Week 5: hybrid MPI+CUDA and performance-regression checks

## Limitations

This educational GEMM code is not a replacement for BLAS. Its purpose is to show transparent design, verification, experiment discipline, and documented optimization decisions.

## License

MIT. See [LICENSE](LICENSE).
