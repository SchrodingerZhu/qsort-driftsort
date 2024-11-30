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

template <class F> void qsort(std::vector<int> a) {
  std::vector<int> b = a;
  std::sort(a.begin(), a.end(), F{});
  std::vector<int> scratch(a.size() + 16);
  driftsort::qsort_r(
      b.data(), sizeof(int), b.size(),
      [](const void *a, const void *b, void *context) {
        return -F{}(*static_cast<const int *>(a), *static_cast<const int *>(b));
      },
      nullptr);
  ASSERT_EQ(a, b);
}

void qsort_less(std::vector<int> a) { qsort<std::less<int>>(a); }

void qsort_greater(std::vector<int> a) { qsort<std::greater<int>>(a); }

FUZZ_TEST(DriftSortTest, qsort_less);
FUZZ_TEST(DriftSortTest, qsort_greater);
