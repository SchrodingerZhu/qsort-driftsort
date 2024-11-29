/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/quicksort.h"
#include "../utils.hpp"
#include "fuzztest/fuzztest.h"
#include <algorithm>
#include <array>
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

using namespace driftsort;
using namespace driftsort::quick;

template <class F> void quick_sort(std::vector<int> a, size_t limit) {
  std::vector<int> b = a;
  std::sort(a.begin(), a.end(), F{});
  std::vector<int> scratch(a.size() + 16);
  stable_quicksort(b.data(), b.size(), scratch.data(), limit, {},
                   compare_blob<int, F>,
                   [](BlobPtr v, size_t length, BlobPtr, auto) {
                     int *base = static_cast<int *>(v.get());
                     std::sort(base, base + length, F{});
                   });
  ASSERT_EQ(a, b);
}

void quick_sort_less(std::vector<int> a, size_t limit) {
  quick_sort<std::less<int>>(a, limit);
}

void quick_sort_greater(std::vector<int> a, size_t limit) {
  quick_sort<std::greater<int>>(a, limit);
}

FUZZ_TEST(DriftSortTest, quick_sort_less)
    .WithDomains(fuzztest::Arbitrary<std::vector<int>>(),
                 fuzztest::InRange(0, 64));

FUZZ_TEST(DriftSortTest, quick_sort_greater)
    .WithDomains(fuzztest::Arbitrary<std::vector<int>>(),
                 fuzztest::InRange(0, 64));
