/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/pivot.h"
#include "../utils.hpp"
#include "driftsort/blob.h"
#include <algorithm>
#include <array>
#include <gtest/gtest.h>
using namespace driftsort;
using namespace driftsort::pivot;

TEST(DriftSortUnitTests, median_of_3) {
  std::array<int, 3> data{1, 2, 3};

  do {
    BlobPtr a = &data[0];
    BlobPtr b = &data[1];
    BlobPtr c = &data[2];

    void *result = median_of_3(a, b, c, compare_blob<int>());
    ASSERT_EQ(*static_cast<int *>(result), 2);
  } while (std::next_permutation(data.begin(), data.end()));
}
