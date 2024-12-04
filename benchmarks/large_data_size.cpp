/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "benchmark_common.h"
#include <array>
#include <benchmark/benchmark.h>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

template <typename QSortImpl>
static void benchmark_qsort_with_large_data_size(benchmark::State &state) {
  size_t n = state.range(0);
  std::vector<std::array<long long, 16>> data(n);
  std::random_device rd;
  std::default_random_engine g(rd());
  std::uniform_int_distribution<long long> dist;
  for (auto &x : data)
    for (auto &y : x)
      y = dist(g);

  for (auto _ : state) {
    state.PauseTiming();
    std::shuffle(data.begin(), data.end(), g);
    state.ResumeTiming();
    QSortImpl::qsort(data.data(), n, sizeof(std::array<long long, 16>),
                     [](const void *a, const void *b) {
                       const auto &x =
                           *static_cast<const std::array<long long, 16> *>(a);
                       const auto &y =
                           *static_cast<const std::array<long long, 16> *>(b);
                       return -(x < y);
                     });
    benchmark::DoNotOptimize(data);
  }
  state.SetBytesProcessed(state.iterations() * n *
                          sizeof(std::array<long long, 16>));
}

BENCHMARK_TEMPLATE(benchmark_qsort_with_large_data_size, driftsort::DriftSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);

BENCHMARK_TEMPLATE(benchmark_qsort_with_large_data_size, driftsort::LibcSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);
