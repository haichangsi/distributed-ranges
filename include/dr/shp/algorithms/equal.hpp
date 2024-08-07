// SPDX-FileCopyrightText: Intel Corporation
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <dr/shp/algorithms/fill.hpp>
#include <dr/shp/algorithms/reduce.hpp>
#include <dr/shp/detail.hpp>
#include <dr/shp/init.hpp>
#include <dr/shp/util.hpp>
#include <dr/shp/views/views.hpp>
#include <dr/shp/zip_view.hpp>

namespace dr::shp {

template <typename ExecutionPolicy, dr::distributed_range R1,
          dr::distributed_range R2>
  requires std::equality_comparable_with<rng::range_value_t<R1>,
                                         rng::range_value_t<R2>>
bool equal(ExecutionPolicy &&policy, R1 &&r1, R2 &&r2) {

  if (rng::distance(r1) != rng::distance(r2)) {
    return false;
  }

  // we must use ints instead of bools, because distributed ranges do not
  // support bools
  auto compare = [](auto &&elems) {
    return elems.first == elems.second ? 1 : 0;
  };

  auto zipped_views = views::zip(r1, r2);
  auto compared = shp::views::transform(zipped_views, compare);
  auto min = [](double x, double y) { return std::min(x, y); };
  auto result = shp::reduce(policy, compared, 1, min);
  return result == 1;
}

template <dr::distributed_range R1, dr::distributed_range R2>
bool equal(R1 &&r1, R2 &&r2) {
  return equal(dr::shp::par_unseq, r1, r2);
}
} // namespace dr::shp
