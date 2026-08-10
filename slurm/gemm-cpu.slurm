#!/bin/bash
#SBATCH --job-name=gemm-bench
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --time=00:10:00
#SBATCH --output=results/slurm-%j.out

set -euo pipefail
module load gcc  # Change for your cluster.
make OPENMP=1
export OMP_NUM_THREADS="$SLURM_CPUS_PER_TASK"
./scripts/run_benchmarks.sh bin/gemm_bench "results/gemm-${SLURM_JOB_ID}.csv"
