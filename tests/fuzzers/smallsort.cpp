/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/smallsort.h"
#include "../utils.hpp"
#include <algorithm>
#include <array>
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

using namespace driftsort;
using namespace driftsort::small;

void sort4_stable_int(std::array<int, 4> in) {
  std::array<int, 4> dst;
  sort4_stable(in.begin(), dst.begin(), compare_blob<int>);
  ASSERT_TRUE(std::is_sorted(dst.begin(), dst.end()));
}

void sort4_stable_int_reverse(std::array<int, 4> in) {
  std::array<int, 4> dst;
  sort4_stable(in.begin(), dst.begin(), compare_blob<int, std::greater<>>);
  ASSERT_TRUE(std::is_sorted(dst.begin(), dst.end(), std::greater<>()));
}

void sort4_is_stable(std::array<int, 4> in) {
  std::array<ElementWithSrc<int>, 4> src{
      ElementWithSrc<int>(in[0]), ElementWithSrc<int>(in[1]),
      ElementWithSrc<int>(in[2]), ElementWithSrc<int>(in[3])};
  std::array<ElementWithSrc<int>, 4> dst;
  for (auto &e : src)
    e.record_address();
  sort4_stable(src.begin(), dst.begin(), compare_blob<int, std::less<>>);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      auto x = dst[i];
      auto y = dst[j];
      if (x == y && x.address_less_eq(y))
        ASSERT_TRUE(i <= j);
    }
  }
}

void bidirectional_merge_test(std::vector<int> a) {
  if (a.size() < 2)
    return;
  auto mid = a.size() / 2;
  std::sort(a.begin(), a.begin() + mid);
  std::sort(a.begin() + mid, a.end());
  std::vector<int> dst(a.size());
  bidirectional_merge(a.data(), a.size(), dst.data(), compare_blob<int>);
  std::sort(a.begin(), a.end());
  ASSERT_EQ(a, dst);
}

void sort8_stable_int(std::array<int, 8> in) {
  std::array<int, 8> dst;
  std::array<int, 8> scratch;
  sort8_stable(in.begin(), dst.begin(), scratch.begin(), compare_blob<int>);
  ASSERT_TRUE(std::is_sorted(dst.begin(), dst.end()));
}

void sort8_stable_int_reverse(std::array<int, 8> in) {
  std::array<int, 8> dst;
  std::array<int, 8> scratch;
  sort8_stable(in.begin(), dst.begin(), scratch.begin(),
               compare_blob<int, std::greater<>>);
  ASSERT_TRUE(std::is_sorted(dst.begin(), dst.end(), std::greater<>()));
}

void sort8_is_stable(std::array<int, 8> in) {
  std::array<ElementWithSrc<int>, 8> src{
      ElementWithSrc<int>(in[0]), ElementWithSrc<int>(in[1]),
      ElementWithSrc<int>(in[2]), ElementWithSrc<int>(in[3]),
      ElementWithSrc<int>(in[4]), ElementWithSrc<int>(in[5]),
      ElementWithSrc<int>(in[6]), ElementWithSrc<int>(in[7])};
  std::array<ElementWithSrc<int>, 8> dst;
  std::array<ElementWithSrc<int>, 8> scratch;
  for (auto &e : src)
    e.record_address();
  sort8_stable(src.begin(), dst.begin(), scratch.begin(), compare_blob<int>);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      auto x = dst[i];
      auto y = dst[j];
      if (x == y && x.address_less_eq(y))
        ASSERT_TRUE(i <= j);
    }
  }
}

void insert_tail_test(std::vector<int> a) {
  if (a.size() < 2)
    return;
  std::vector<int> b = a;
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end() - 1);
  insert_tail(b.data(), b.data() + b.size() - 1, compare_blob<int>);
  ASSERT_EQ(a, b);
}

void insertion_sort_shift_left_test(std::vector<int> a, size_t offset) {
  if (a.size() < 2)
    return;
  std::vector<int> b = a;
  std::sort(a.begin(), a.end());
  offset = offset % (a.size() - 1) + 1;
  std::sort(b.begin(), b.begin() + offset);
  insertion_sort_shift_left(b.data(), b.size(), offset, compare_blob<int>);
  ASSERT_EQ(a, b);
}

void small_sort_general_test(std::vector<int> a) {
  std::vector<int> b = a;
  std::vector<int> scratch_pad(a.size() + 16);
  std::sort(a.begin(), a.end());
  small_sort_general(b.data(), b.size(), scratch_pad.data(), compare_blob<int>);
  ASSERT_EQ(a, b);
}

void small_sort_general_large_data_test(
    std::vector<std::array<long long, 8>> a) {
  std::vector b = a;
  std::vector scratch_pad(a.size() + 16, std::array<long long, 8>{});
  std::sort(a.begin(), a.end());
  small_sort_general(b.data(), b.size(), scratch_pad.data(),
                     compare_blob<std::array<long long, 8>>);
  ASSERT_EQ(a, b);
}

FUZZ_TEST(DriftSortTest, sort4_stable_int);
FUZZ_TEST(DriftSortTest, sort4_stable_int_reverse);
FUZZ_TEST(DriftSortTest, sort4_is_stable);
FUZZ_TEST(DriftSortTest, bidirectional_merge_test);
FUZZ_TEST(DriftSortTest, sort8_stable_int);
FUZZ_TEST(DriftSortTest, sort8_stable_int_reverse);
FUZZ_TEST(DriftSortTest, sort8_is_stable);
FUZZ_TEST(DriftSortTest, insert_tail_test);
FUZZ_TEST(DriftSortTest, insertion_sort_shift_left_test);
FUZZ_TEST(DriftSortTest, small_sort_general_test);
FUZZ_TEST(DriftSortTest, small_sort_general_large_data_test);
