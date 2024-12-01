/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "symbols.h"
#include <cstdlib>

namespace driftsort {
struct DriftSort {
  static void qsort(void *base, size_t nmemb, size_t size, compar_fn_t compar) {
    driftsort_qsort(base, nmemb, size, compar);
  }
};
struct LibcSort {
  static void qsort(void *base, size_t nmemb, size_t size, compar_fn_t compar) {
    ::qsort(base, nmemb, size, compar);
  }
};
} // namespace driftsort
