/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "benchmark_common.h"
#include <benchmark/benchmark.h>
#include <cstdlib>
#include <numeric>
#include <vector>

template <typename QSortImpl>
static void benchmark_qsort_on_sorted_sequence(benchmark::State &state) {
  size_t n = state.range(0);
  std::vector<int> data(n);
  std::iota(data.begin(), data.end(), 0);
  for (auto _ : state) {
    QSortImpl::qsort(
        data.data(), sizeof(int), n, [](const void *a, const void *b) {
          return *static_cast<const int *>(a) - *static_cast<const int *>(b);
        });
    benchmark::DoNotOptimize(data);
  }
  state.SetBytesProcessed(state.iterations() * n * sizeof(int));
}

BENCHMARK_TEMPLATE(benchmark_qsort_on_sorted_sequence, driftsort::DriftSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);

BENCHMARK_TEMPLATE(benchmark_qsort_on_sorted_sequence, driftsort::LibcSort)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 16);
