#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef HPC_PERF_USE_OPENMP
#include <omp.h>
#endif

namespace {

struct Options {
  int size = 512;
  int block_size = 32;
  int repetitions = 3;
  int threads = 1;
  bool verify = false;
  bool csv = false;
};

int parse_positive(const char* value, const std::string& option) {
  try {
    const int parsed = std::stoi(value);
    if (parsed <= 0) throw std::invalid_argument("not positive");
    return parsed;
  } catch (const std::exception&) {
    throw std::runtime_error(option + " must be a positive integer");
  }
}

Options parse_options(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const auto read_value = [&](const std::string& name) {
      if (++i >= argc) throw std::runtime_error("missing value for " + name);
      return parse_positive(argv[i], name);
    };
    if (arg == "--size") options.size = read_value(arg);
    else if (arg == "--block-size") options.block_size = read_value(arg);
    else if (arg == "--repetitions") options.repetitions = read_value(arg);
    else if (arg == "--threads") options.threads = read_value(arg);
    else if (arg == "--verify") options.verify = true;
    else if (arg == "--csv") options.csv = true;
    else if (arg == "--help") {
      std::cout << "Usage: gemm_bench [--size N] [--block-size N] [--repetitions N] "
                   "[--threads N] [--verify] [--csv]\n";
      std::exit(EXIT_SUCCESS);
    } else {
      throw std::runtime_error("unknown option: " + arg);
    }
  }
  return options;
}

using Matrix = std::vector<double>;

void fill_deterministic(Matrix& matrix) {
  std::mt19937 generator(42);
  std::uniform_real_distribution<double> distribution(-1.0, 1.0);
  for (double& value : matrix) value = distribution(generator);
}

void gemm_naive(const Matrix& a, const Matrix& b, Matrix& c, int n) {
  std::fill(c.begin(), c.end(), 0.0);
  for (int i = 0; i < n; ++i)
    for (int k = 0; k < n; ++k) {
      const double a_ik = a[static_cast<size_t>(i) * n + k];
      for (int j = 0; j < n; ++j)
        c[static_cast<size_t>(i) * n + j] += a_ik * b[static_cast<size_t>(k) * n + j];
    }
}

void gemm_blocked(const Matrix& a, const Matrix& b, Matrix& c, int n, int block_size,
                  int threads) {
  std::fill(c.begin(), c.end(), 0.0);
#ifdef HPC_PERF_USE_OPENMP
  omp_set_num_threads(threads);
#pragma omp parallel for collapse(2) schedule(static)
#else
  (void)threads;
#endif
  for (int ii = 0; ii < n; ii += block_size) {
    for (int jj = 0; jj < n; jj += block_size) {
      for (int kk = 0; kk < n; kk += block_size) {
        const int i_end = std::min(ii + block_size, n);
        const int j_end = std::min(jj + block_size, n);
        const int k_end = std::min(kk + block_size, n);
        for (int i = ii; i < i_end; ++i)
          for (int k = kk; k < k_end; ++k) {
            const double a_ik = a[static_cast<size_t>(i) * n + k];
            for (int j = jj; j < j_end; ++j)
              c[static_cast<size_t>(i) * n + j] += a_ik * b[static_cast<size_t>(k) * n + j];
          }
      }
    }
  }
}

bool approximately_equal(const Matrix& lhs, const Matrix& rhs) {
  for (size_t i = 0; i < lhs.size(); ++i)
    if (std::abs(lhs[i] - rhs[i]) > 1e-9) return false;
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const Options options = parse_options(argc, argv);
    const size_t element_count = static_cast<size_t>(options.size) * options.size;
    Matrix a(element_count), b(element_count), c(element_count), reference(element_count);
    fill_deterministic(a);
    fill_deterministic(b);

    if (options.verify) gemm_naive(a, b, reference, options.size);

    double elapsed_seconds = 0.0;
    for (int run = 0; run < options.repetitions; ++run) {
      const auto start = std::chrono::steady_clock::now();
      gemm_blocked(a, b, c, options.size, options.block_size, options.threads);
      elapsed_seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    }
    const double average_seconds = elapsed_seconds / options.repetitions;
    const double gflops = (2.0 * options.size * options.size * options.size) /
                          (average_seconds * 1.0e9);
    const bool valid = !options.verify || approximately_equal(c, reference);

    if (options.csv) {
      std::cout << "size,block_size,repetitions,threads,avg_seconds,gflops,verified\n"
                << options.size << ',' << options.block_size << ',' << options.repetitions << ','
                << options.threads << ',' << std::fixed << std::setprecision(6) << average_seconds
                << ',' << std::setprecision(3) << gflops << ',' << (valid ? "true" : "false") << '\n';
    } else {
      std::cout << "GEMM benchmark\nsize: " << options.size << " x " << options.size
                << "\nblock size: " << options.block_size << "\nthreads: " << options.threads
                << "\naverage runtime: " << std::fixed << std::setprecision(6) << average_seconds
                << " s\nthroughput: " << std::setprecision(3) << gflops << " GFLOP/s\nverification: "
                << (valid ? "PASS" : "FAIL") << '\n';
    }
    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << "\nTry --help for usage.\n";
    return EXIT_FAILURE;
  }
}
