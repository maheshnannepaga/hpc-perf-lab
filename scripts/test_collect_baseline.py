import subprocess
import unittest
from unittest.mock import patch
from collect_baseline import sample


class CollectorTests(unittest.TestCase):
    def response(self, verified='true', size=511):
        return subprocess.CompletedProcess([], 0,
            'size,block_size,repetitions,threads,avg_seconds,gflops,verified\n'
            f'{size},32,3,1,0.010000,26.686,{verified}\n')

    @patch('collect_baseline.subprocess.run')
    def test_requests_verification_and_accepts_non_aligned_size(self, run):
        run.return_value = self.response()
        self.assertEqual(sample('bench', 511, 32, 3)['size'], '511')
        self.assertIn('--verify', run.call_args.args[0])

    @patch('collect_baseline.subprocess.run')
    def test_rejects_failed_verification(self, run):
        run.return_value = self.response(verified='false')
        with self.assertRaises(ValueError):
            sample('bench', 511, 32, 3)

    @patch('collect_baseline.subprocess.run')
    def test_rejects_wrong_workload(self, run):
        run.return_value = self.response(size=512)
        with self.assertRaises(ValueError):
            sample('bench', 511, 32, 3)

    @patch('collect_baseline.subprocess.run')
    def test_propagates_program_failure(self, run):
        run.side_effect = subprocess.CalledProcessError(1, ['bench'])
        with self.assertRaises(subprocess.CalledProcessError):
            sample('bench', 511, 32, 3)


if __name__ == '__main__':
    unittest.main()
