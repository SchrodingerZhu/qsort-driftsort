/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "driftsort/driftsort.h"

#ifdef __APPLE__
typedef int (*compar_d_fn_t)(void *, const void *, const void *);
typedef int (*compar_fn_t)(const void *, const void *);
extern "C" void qsort_r(void *base, size_t nmemb, size_t size, void *arg,
                        compar_d_fn_t compar) {
  driftsort::qsort_r(base, nmemb, size,
                     [compar, arg](const void *a, const void *b) {
                       return compar(arg, a, b);
                     });
}
#else
typedef int (*compar_d_fn_t)(const void *, const void *, void *);
typedef int (*compar_fn_t)(const void *, const void *);
extern "C" void qsort_r(void *base, size_t nmemb, size_t size,
                        compar_d_fn_t compar, void *arg) {
  driftsort::qsort_r(base, nmemb, size,
                     [compar, arg](const void *a, const void *b) {
                       return compar(a, b, arg);
                     });
}
#endif
extern "C" void qsort(void *base, size_t nmemb, size_t size,
                      compar_fn_t compar) {
  driftsort::qsort_r(base, nmemb, size, compar);
}
