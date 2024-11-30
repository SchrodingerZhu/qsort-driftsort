
#include "driftsort/driftsort.h"
#include <cstdlib>

extern "C" void qsort_r(void *base, size_t nmemb, size_t size,
                        __compar_d_fn_t compar, void *arg) {
  driftsort::qsort_r(base, nmemb, size, compar, arg);
}
