
#include "driftsort/driftsort.h"
#include <cstdlib>

typedef int (*compar_d_fn_t)(const void *, const void *, void *);
typedef int (*compar_fn_t)(const void *, const void *);

extern "C" void qsort_r(void *base, size_t nmemb, size_t size,
                        compar_d_fn_t compar, void *arg) {
  driftsort::qsort_r(base, nmemb, size,
                     [compar, arg](const void *a, const void *b) {
                       return compar(a, b, arg);
                     });
}

extern "C" void qsort(void *base, size_t nmemb, size_t size,
                      compar_fn_t compar) {
  driftsort::qsort_r(base, nmemb, size, compar);
}
