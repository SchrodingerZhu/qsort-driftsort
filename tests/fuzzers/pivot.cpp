/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/pivot.h"
#include "../utils.hpp"
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>
#include <vector>
using namespace driftsort;
using namespace driftsort::pivot;

void pivot_selection(std::vector<int> data) {
  if (data.size() < 3)
    return;
  size_t pivot = choose_pivot(data.data(), data.size(), compare_blob<int>());
  ASSERT_LT(pivot, data.size());
}

FUZZ_TEST(DriftSortTest, pivot_selection);
