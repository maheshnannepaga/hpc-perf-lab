"""Collect verified serial GEMM samples; Python standard library only."""
import argparse
import csv
import hashlib
import io
import json
import platform
from pathlib import Path
import subprocess
from datetime import datetime, timezone


def sample(binary, size, block, repetitions):
    command = [str(binary), '--size', str(size), '--block-size', str(block),
               '--repetitions', str(repetitions), '--threads', '1', '--verify', '--csv']
    completed = subprocess.run(command, check=True, capture_output=True, text=True)
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    if len(rows) != 1 or rows[0]['verified'] != 'true':
        raise ValueError('Expected exactly one verified benchmark row')
    row = rows[0]
    if (int(row['size']), int(row['block_size']), int(row['threads'])) != (size, block, 1):
        raise ValueError('Benchmark returned a different workload')
    if float(row['avg_seconds']) <= 0 or float(row['gflops']) <= 0:
        raise ValueError('Timing resolution insufficient for this workload')
    return row


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=Path('bin/gemm_bench'))
    parser.add_argument('--output', type=Path, required=True,
                        help='New directory; existing results are never overwritten')
    parser.add_argument('--source-revision', required=True)
    parser.add_argument('--build-command', required=True)
    parser.add_argument('--compiler', default='c++')
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    args.output.mkdir(parents=True, exist_ok=False)
    metadata = {
        'timestamp_utc': datetime.now(timezone.utc).isoformat(),
        'system': platform.system(), 'release': platform.release(),
        'machine': platform.machine(),
        'processor': platform.processor() or 'not exposed',
        'source_revision': args.source_revision,
        'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
        'build_command': args.build_command,
        'compiler_version': subprocess.run([args.compiler, '--version'], check=True,
                                           capture_output=True, text=True).stdout,
        'openmp': False, 'threads': 1, 'verification_requested': True,
        'design': 'one warm-up per configuration, five process samples, three repetitions per sample',
        'limitations': 'Local CPU only. CPU model/core count unavailable through this harness. '
                      'No affinity or power control; six-decimal-second timer output. '
                      'No naive/BLAS baseline, CUDA, MPI, or Slurm execution.'
    }
    (args.output / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
    rows = []
    for size in (256, 511, 512):
        for block in (16, 32, 64):
            sample(binary, size, block, 1)  # unreported warm-up
            for index in range(5):
                rows.append({'sample': index + 1, **sample(binary, size, block, 3)})
    with (args.output / 'samples.csv').open('w', newline='') as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print('Wrote', len(rows), 'verified serial CPU samples to', args.output)


if __name__ == '__main__':
    main()
