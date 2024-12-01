/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/driftsort.h"

#include "symbols.h"

extern "C" void driftsort_qsort_r(void *base, size_t nmemb, size_t size,
                                  compar_d_fn_t compar, void *arg) {
  driftsort::qsort_r(base, nmemb, size,
                     [compar, arg](const void *a, const void *b) {
                       return compar(a, b, arg);
                     });
}
extern "C" void driftsort_qsort(void *base, size_t nmemb, size_t size,
                                compar_fn_t compar) {
  driftsort::qsort_r(base, nmemb, size, compar);
}
