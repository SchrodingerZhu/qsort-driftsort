/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>
#include <vector>

void simple_sort(std::vector<int> v) {
  std::sort(v.begin(), v.end());
  ASSERT_TRUE(std::is_sorted(v.begin(), v.end()));
}

FUZZ_TEST(DriftSortTest, simple_sort);
