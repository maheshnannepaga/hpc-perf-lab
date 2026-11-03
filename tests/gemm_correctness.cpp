// Exercise the actual kernels, including partial tiles and invalid results.
#define main benchmark_cli_main
#include "../src/gemm_bench.cpp"
#undef main
#include <limits>

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int main() {
  // An independently calculated, non-symmetric product catches indexing errors.
  Matrix a{1, 2, 3, 4}, b{5, 6, 7, 8}, expected{19, 22, 43, 50};
  for (int tile : {1, 2, 3}) {
    Matrix c(4, -99.0);
    gemm_blocked(a, b, c, 2, tile, 1);
    require(approximately_equal(c, expected), "known product mismatch");
    gemm_blocked(a, b, c, 2, tile, 1);
    require(approximately_equal(c, expected), "output was not reset");
  }
  for (int n : {1, 3, 17, 33}) {
    Matrix x(n * n), y(n * n), reference(n * n), c(n * n);
    for (int i = 0; i < n * n; ++i) {
      x[i] = (i % 11 - 5) * 0.25;
      y[i] = (i % 7 - 3) * 0.5;
    }
    gemm_naive(x, y, reference, n);
    for (int tile : {1, 4, 16, 64}) {
      gemm_blocked(x, y, c, n, tile, 1);
      require(approximately_equal(c, reference), "partial tile mismatch");
    }
  }
  require(!approximately_equal({1}, {1, 2}), "shape mismatch accepted");
  require(!approximately_equal({1}, {2}), "incorrect value accepted");
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  require(!approximately_equal({nan}, {0}), "NaN accepted");
  require(!approximately_equal({0}, {nan}), "reference NaN accepted");
  require(!approximately_equal({inf}, {inf}), "infinity accepted");
  std::cout << "Kernel correctness checks passed\n";
}
