/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>

typedef int (*compar_d_fn_t)(const void *, const void *, void *);
typedef int (*compar_fn_t)(const void *, const void *);

extern "C" void driftsort_qsort_r(void *base, size_t nmemb, size_t size,
                                  compar_d_fn_t compar, void *arg);

extern "C" void driftsort_qsort(void *base, size_t nmemb, size_t size,
                                compar_fn_t compar);
