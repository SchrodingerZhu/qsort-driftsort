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
  driftsort::qsort_r(
      b.data(), sizeof(int), b.size(), [](const void *a, const void *b) {
        return -F{}(*static_cast<const int *>(a), *static_cast<const int *>(b));
      });
  ASSERT_EQ(a, b);
}

void qsort_less(std::vector<int> a) { qsort<std::less<int>>(a); }

void qsort_greater(std::vector<int> a) { qsort<std::greater<int>>(a); }

template <size_t Factor>
struct alignas(alignof(long) * Factor) OverAlignedLong {
  std::array<long, Factor> data;
  constexpr OverAlignedLong(std::array<long, Factor> data) : data(data) {}
  bool operator<(const OverAlignedLong &other) const {
    return data < other.data;
  }
  bool operator==(const OverAlignedLong &other) const {
    return data == other.data;
  }
};

template <size_t Factor>
void qsort_over_aligned(std::vector<std::array<long, Factor>> a) {
  std::vector<OverAlignedLong<Factor>> b;
  for (auto &e : a)
    b.push_back(OverAlignedLong<Factor>{e});
  std::vector<OverAlignedLong<Factor>> c = b;
  std::sort(b.begin(), b.end());
  driftsort::qsort_r(c.data(), sizeof(OverAlignedLong<Factor>), c.size(),
                     [](const void *a, const void *b) {
                       return -(
                           *static_cast<const OverAlignedLong<Factor> *>(a) <
                           *static_cast<const OverAlignedLong<Factor> *>(b));
                     });
  ASSERT_EQ(b, c);
}

void qsort_over_aligned_2(std::vector<std::array<long, 2>> a) {
  qsort_over_aligned<2>(std::move(a));
}

void qsort_over_aligned_4(std::vector<std::array<long, 4>> a) {
  qsort_over_aligned<4>(std::move(a));
}

void qsort_over_aligned_8(std::vector<std::array<long, 8>> a) {
  qsort_over_aligned<8>(std::move(a));
}

FUZZ_TEST(DriftSortTest, qsort_less);
FUZZ_TEST(DriftSortTest, qsort_greater);
FUZZ_TEST(DriftSortTest, qsort_over_aligned_2);
FUZZ_TEST(DriftSortTest, qsort_over_aligned_4);
FUZZ_TEST(DriftSortTest, qsort_over_aligned_8);
