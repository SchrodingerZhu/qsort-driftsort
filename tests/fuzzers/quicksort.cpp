/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../utils.hpp"
#include "driftsort/driftsort.h"
#include "fuzztest/fuzztest.h"
#include <algorithm>
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

using namespace driftsort;
using namespace driftsort::quick;

template <class F> void quick_sort(std::vector<int> a, size_t limit) {
  std::vector<int> b = a;
  std::sort(a.begin(), a.end(), F{});
  std::vector<int> scratch(a.size() + 16);
  stable_quicksort(b.data(), b.size(), scratch.data(), scratch.size(), limit,
                   {}, compare_blob<int, F>());
  ASSERT_EQ(a, b);
}

void quick_sort_less(std::vector<int> a, size_t limit) {
  quick_sort<std::less<int>>(a, limit);
}

void quick_sort_greater(std::vector<int> a, size_t limit) {
  quick_sort<std::greater<int>>(a, limit);
}

void quick_sort_is_stable(std::vector<int> a, size_t limit) {
  std::vector<ElementWithSrc<int>> src{};
  for (auto &e : a)
    src.push_back(ElementWithSrc<int>(e));
  for (auto &e : src)
    e.record_address();
  std::vector<ElementWithSrc<int>> scratch(src.size() + 16);
  stable_quicksort(src.data(), src.size(), scratch.data(), scratch.size(),
                   limit, {}, compare_blob<ElementWithSrc<int>>());
  for (size_t i = 0; i < src.size(); i++) {
    for (size_t j = 0; j < src.size(); j++) {
      auto x = src[i];
      auto y = src[j];
      if (x == y && x.address_less_eq(y))
        ASSERT_TRUE(i <= j);
    }
  }
}

FUZZ_TEST(DriftSortTest, quick_sort_less)
    .WithDomains(fuzztest::Arbitrary<std::vector<int>>(),
                 fuzztest::InRange(0, 64));

FUZZ_TEST(DriftSortTest, quick_sort_greater)
    .WithDomains(fuzztest::Arbitrary<std::vector<int>>(),
                 fuzztest::InRange(0, 64));

FUZZ_TEST(DriftSortTest, quick_sort_is_stable)
    .WithDomains(fuzztest::Arbitrary<std::vector<int>>(),
                 fuzztest::InRange(0, 64));
