// SPDX-FileCopyrightText: Intel Corporation
//
// SPDX-License-Identifier: BSD-3-Clause

#include "xp-tests.hpp"
auto testMatrixIter(auto &src, auto &matrix) {
  EXPECT_TRUE(src.size() == matrix.size());
  std::map<std::pair<long, long>, double> entries;
  for (auto [index, val] : src) {
    entries[{index.first, index.second}] = val;
  }
  for (auto [index, val] : matrix) {
    EXPECT_TRUE((val == entries[{index.first, index.second}]));
  }
}

auto testMatrixReduce(auto &src, auto &matrix) {
  EXPECT_TRUE(src.size() == matrix.size());
  long sum = 0;
  for (auto [index, val] : src) {
    auto [x, y] = index;
    sum += (long)(val + x + y);
  }
  auto transformer = [](auto entry) {
    auto [index, val] = entry;
    auto [x, y] = index;
    return (long)(val + x + y);
  };
  auto transformed = dr::transform_view(matrix, transformer);
  long reduced = dr::mp::reduce(transformed, 0, std::plus<long>{});
  EXPECT_TRUE((sum == reduced));
}

TEST(SparseMatrix, staticAssertEq) {
  std::size_t m = 100;
  std::size_t k = 100;
  using Dist =
      dr::mp::csr_eq_distribution<float, unsigned long, dr::mp::MpiBackend>;
  static_assert(dr::mp::matrix_distibution<Dist>);
  static_assert(dr::mp::vector_multiplicable<Dist>);
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<float, unsigned long, dr::mp::MpiBackend> a(
      csr, 0);
  static_assert(std::forward_iterator<decltype(a.begin())>);
  static_assert(std::forward_iterator<decltype(a.end())>);
  static_assert(std::forward_iterator<decltype(rng::begin(a))>);
  static_assert(std::forward_iterator<decltype(rng::end(a))>);
  using Matrix = decltype(a);
  static_assert(rng::forward_range<Matrix>);
  static_assert(dr::distributed_range<Matrix>);
}

TEST(SparseMatrix, staticAssertRow) {
  std::size_t m = 100;
  std::size_t k = 100;
  using Dist =
      dr::mp::csr_row_distribution<float, unsigned long, dr::mp::MpiBackend>;
  static_assert(dr::mp::matrix_distibution<Dist>);
  static_assert(dr::mp::vector_multiplicable<Dist>);
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<float, unsigned long, dr::mp::MpiBackend> a(
      csr, 0);
  static_assert(std::forward_iterator<decltype(a.begin())>);
  static_assert(std::forward_iterator<decltype(a.end())>);
  static_assert(std::forward_iterator<decltype(rng::begin(a))>);
  static_assert(std::forward_iterator<decltype(rng::end(a))>);
  using Matrix = decltype(a);
  static_assert(rng::forward_range<Matrix>);
  static_assert(dr::distributed_range<Matrix>);
}

TEST(SparseMatrix, IterRow) {
  std::size_t m = 100;
  std::size_t k = 100;
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<
      float, unsigned long, dr::mp::MpiBackend,
      dr::mp::csr_row_distribution<float, unsigned long, dr::mp::MpiBackend>>
      a(csr, 0);
  testMatrixIter(csr, a);
}

TEST(SparseMatrix, IterEq) {
  std::size_t m = 100;
  std::size_t k = 100;
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<
      float, unsigned long, dr::mp::MpiBackend,
      dr::mp::csr_eq_distribution<float, unsigned long, dr::mp::MpiBackend>>
      a(csr, 0);
  testMatrixIter(csr, a);
}

TEST(SparseMatrix, ReduceRow) {
  std::size_t m = 100;
  std::size_t k = 100;
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<
      float, unsigned long, dr::mp::MpiBackend,
      dr::mp::csr_row_distribution<float, unsigned long, dr::mp::MpiBackend>>
      a(csr, 0);
  testMatrixReduce(csr, a);
}

TEST(SparseMatrix, ReduceEq) {
  std::size_t m = 100;
  std::size_t k = 100;
  auto csr = dr::generate_random_csr({m, k}, 0.1f);
  dr::mp::distributed_sparse_matrix<
      float, unsigned long, dr::mp::MpiBackend,
      dr::mp::csr_eq_distribution<float, unsigned long, dr::mp::MpiBackend>>
      a(csr, 0);
  testMatrixReduce(csr, a);
}
