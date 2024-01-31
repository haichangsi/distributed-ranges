// SPDX-FileCopyrightText: Intel Corporation
//
// SPDX-License-Identifier: BSD-3-Clause

#include "xhp-tests.hpp"

template <typename T> class DistributedVector3 : public testing::Test {};

using T = int;
using DV = dr::mhp::distributed_vector<T>;
using DVI = typename DV::iterator;

TYPED_TEST_SUITE(DistributedVector3, AllTypes);

TYPED_TEST(DistributedVector3, suite_works_for_3_processes_only) {
  EXPECT_EQ(dr::mhp::default_comm().size(), 3);
}

TYPED_TEST(DistributedVector3, DistributedVectorSegmentsValues) {
  std::size_t segment_size = 4;
  std::size_t n = 30;
  auto dist = dr::mhp::distribution();
  DV dv(n, dist, segment_size);

  iota(dv.begin(), dv.end(), 1);

  if (dr::mhp::default_comm().rank() == 0) {

    EXPECT_EQ(*(dv.begin() + 0).local(), 1);
    EXPECT_EQ(*(dv.begin() + 1).local(), 2);
    EXPECT_EQ(*(dv.begin() + 2).local(), 3);
    EXPECT_EQ(*(dv.begin() + 3).local(), 4);

    EXPECT_EQ(*(dv.begin() + 12).local(), 13);
    EXPECT_EQ(*(dv.begin() + 13).local(), 14);
    EXPECT_EQ(*(dv.begin() + 14).local(), 15);
    EXPECT_EQ(*(dv.begin() + 15).local(), 16);

    EXPECT_EQ(*(dv.begin() + 24).local(), 25);
    EXPECT_EQ(*(dv.begin() + 25).local(), 26);
    EXPECT_EQ(*(dv.begin() + 26).local(), 27);
    EXPECT_EQ(*(dv.begin() + 27).local(), 28);

  } else if (dr::mhp::default_comm().rank() == 1) {
    EXPECT_EQ(*(dv.begin() + 4).local(), 5);
    EXPECT_EQ(*(dv.begin() + 5).local(), 6);
    EXPECT_EQ(*(dv.begin() + 6).local(), 7);
    EXPECT_EQ(*(dv.begin() + 7).local(), 8);

    EXPECT_EQ(*(dv.begin() + 16).local(), 17);
    EXPECT_EQ(*(dv.begin() + 17).local(), 18);
    EXPECT_EQ(*(dv.begin() + 18).local(), 19);
    EXPECT_EQ(*(dv.begin() + 19).local(), 20);

    EXPECT_EQ(*(dv.begin() + 28).local(), 29);

  } else {
    assert(dr::mhp::default_comm().rank() == 2);

    EXPECT_EQ(*(dv.begin() + 8).local(), 9);
    EXPECT_EQ(*(dv.begin() + 9).local(), 10);
    EXPECT_EQ(*(dv.begin() + 8).local(), 11);
    EXPECT_EQ(*(dv.begin() + 9).local(), 12);

    EXPECT_EQ(*(dv.begin() + 20).local(), 21);
    EXPECT_EQ(*(dv.begin() + 21).local(), 22);
    EXPECT_EQ(*(dv.begin() + 22).local(), 23);
    EXPECT_EQ(*(dv.begin() + 23).local(), 24);
  }
}

TYPED_TEST(DistributedVector3, DistributedVectorSegmentsSize) {
  std::size_t segment_size = 4;
  std::size_t n = 30;
  auto dist = dr::mhp::distribution();
  DV dv(n, dist, segment_size);

  iota(dv.begin(), dv.end(), 1);

  EXPECT_EQ(dv.segments().size(), 8);
  EXPECT_EQ(rng::size(dv.segments()[0]), segment_size);
}
