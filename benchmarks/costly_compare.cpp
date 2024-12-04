/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "benchmark_common.h"
#include <benchmark/benchmark.h>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

template <typename QSortImpl>
static void benchmark_qsort_with_costly_compare(benchmark::State &state) {
  size_t n = state.range(0);
  std::vector<int> data(n);
  std::iota(data.begin(), data.end(), 0);
  std::random_device rd;
  std::default_random_engine g(rd());
  for (auto _ : state) {
    QSortImpl::qsort(data.data(), n, sizeof(int),
                     [](const void *a, const void *b) {
                       int x = *static_cast<const int *>(a);
                       int y = *static_cast<const int *>(b);
                       long double s = static_cast<long double>(x);
                       long double t = static_cast<long double>(y);
                       long double u = std::log2(s + 1.0L);
                       long double v = std::log2(t + 1.0L);
                       return -(u < v);
                     });
    benchmark::DoNotOptimize(data);
  }
  state.SetBytesProcessed(state.iterations() * n * sizeof(int));
}

BENCHMARK_TEMPLATE(benchmark_qsort_with_costly_compare, driftsort::DriftSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);

BENCHMARK_TEMPLATE(benchmark_qsort_with_costly_compare, driftsort::LibcSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);
