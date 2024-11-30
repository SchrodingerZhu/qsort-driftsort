/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/merge.h"
#include "../utils.hpp"
#include "fuzztest/fuzztest.h"
#include <algorithm>
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>
#include <vector>

using namespace driftsort;

void merge_test(std::vector<int> a, size_t mid) {
  std::vector<int> b = a;
  std::sort(a.begin(), a.end());
  std::vector<int> scratch(a.size() + 16);
  mid = mid % (a.size() + 1);
  std::sort(b.begin(), b.begin() + mid);
  std::sort(b.begin() + mid, b.end());
  merge::merge(b.data(), b.size(), scratch.data(), scratch.size(), mid,
               compare_blob<int>());
  ASSERT_EQ(a, b);
}

void merge_test_greater(std::vector<int> a, size_t mid) {
  std::vector<int> b = a;
  std::sort(a.begin(), a.end(), std::greater<int>());
  std::vector<int> scratch(a.size() + 16);
  mid = mid % (a.size() + 1);
  std::sort(b.begin(), b.begin() + mid, std::greater<int>());
  std::sort(b.begin() + mid, b.end(), std::greater<int>());
  merge::merge(b.data(), b.size(), scratch.data(), scratch.size(), mid,
               compare_blob<int, std::greater<int>>());
  ASSERT_EQ(a, b);
}

void merge_is_stable(std::vector<int> a, size_t mid) {
  std::vector<ElementWithSrc<int>> src{};
  for (auto &e : a)
    src.push_back(ElementWithSrc<int>(e));
  for (auto &e : src)
    e.record_address();
  std::vector<ElementWithSrc<int>> scratch(src.size() + 16);
  mid = mid % (a.size() + 1);
  std::stable_sort(src.begin(), src.begin() + mid);
  std::stable_sort(src.begin() + mid, src.end());
  merge::merge(src.data(), src.size(), scratch.data(), scratch.size(), mid,
               compare_blob<ElementWithSrc<int>>());
  for (size_t i = 0; i < src.size(); i++) {
    for (size_t j = 0; j < src.size(); j++) {
      auto x = src[i];
      auto y = src[j];
      if (x == y && x.address_less_eq(y))
        ASSERT_TRUE(i <= j);
    }
  }
}

FUZZ_TEST(DriftSortTest, merge_test);
FUZZ_TEST(DriftSortTest, merge_test_greater);
FUZZ_TEST(DriftSortTest, merge_is_stable);
