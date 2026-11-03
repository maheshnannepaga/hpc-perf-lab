"""Run against the serial binary: python3 tests/test_cli_verification.py BIN."""
import csv
import io
import subprocess
import sys
import unittest

BINARY = sys.argv.pop(1) if len(sys.argv) > 1 else 'bin/gemm_bench'


class VerificationCLI(unittest.TestCase):
    def run_bench(self, *args):
        return subprocess.run([BINARY, '--size', '17', '--block-size', '4',
                               '--repetitions', '1', *args],
                              capture_output=True, text=True)

    def test_without_verification_is_explicit(self):
        result = self.run_bench()
        self.assertEqual(result.returncode, 0)
        self.assertIn('verification: NOT REQUESTED', result.stdout)

    def test_csv_distinguishes_unchecked_from_pass(self):
        result = self.run_bench('--csv')
        self.assertEqual(result.returncode, 0)
        self.assertEqual(list(csv.DictReader(io.StringIO(result.stdout)))[0]
                         ['verified'], 'not_requested')

    def test_verified_partial_tile_passes(self):
        result = self.run_bench('--csv', '--verify')
        self.assertEqual(result.returncode, 0)
        self.assertEqual(list(csv.DictReader(io.StringIO(result.stdout)))[0]
                         ['verified'], 'true')

    def test_serial_rejects_multithread_label(self):
        result = self.run_bench('--threads', '2')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('OpenMP build', result.stderr)

    def test_malformed_integer_rejected(self):
        for value in ['17garbage', '1.5', '-1', '0', '99999999999999999999']:
            with self.subTest(value=value):
                self.assertNotEqual(self.run_bench('--size', value).returncode, 0)


if __name__ == '__main__':
    unittest.main()
