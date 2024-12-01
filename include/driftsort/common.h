/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cassert>

#if __has_builtin(__assume)
#define DRIFTSORT_ASSUME_(x) __assume(x)
#elif __has_builtin(__builtin_assume)
#define DRIFTSORT_ASSUME_(x) __builtin_assume(x)
#elif __has_builtin(__builtin_unreachable)
#define DRIFTSORT_ASSUME_(x)                                                   \
  do {                                                                         \
    if (!(x))                                                                  \
      __builtin_unreachable();                                                 \
  } while (0)
#else
#define DRIFTSORT_ASSUME_(x) (void)(x)
#endif

#ifdef NDEBUG
#define DRIFTSORT_ASSUME(x) DRIFTSORT_ASSUME_(x)
#else
#define DRIFTSORT_ASSUME(x) assert(x)
#endif

#ifdef __GNUC__
#define DRIFTSORT_LIKELY(x) __builtin_expect(!!(x), 1)
#define DRIFTSORT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define DRIFTSORT_CONST [[gnu::const]]
#else
#define DRIFTSORT_LIKELY(x) (x)
#define DRIFTSORT_UNLIKELY(x) (x)
#define DRIFTSORT_CONST
#endif

#if __has_builtin(__builtin_alloca)
#define DRIFTSORT_ALLOCA(comp, n)                                              \
  static_cast<std::byte *>(                                                    \
      __builtin_alloca(comp.size() * n + comp.alloca_padding()))
#elif defined(_MSC_VER)
#define DRIFTSORT_ALLOCA(comp, n)                                              \
  static_cast<std::byte *>(_malloca(comp.size() * n + comp.alloca_padding()))
#endif

#if __has_attribute(visibility)
#define DRIFTSORT_HIDDEN [[gnu::visibility("hidden")]]
#define DRIFTSORT_EXPORT [[gnu::visibility("default")]]
#else
#define DRIFTSORT_HIDDEN
#define DRIFTSORT_EXPORT
#endif

#if __has_attribute(uninitialized)
#define DRIFTSORT_UNINITIALIZED [[clang::uninitialized]]
#else
#define DRIFTSORT_UNINITIALIZED
#endif

#if __has_attribute(noinline)
#define DRIFTSORT_NOINLINE [[gnu::noinline]]
#else
#define DRIFTSORT_NOINLINE
#endif

namespace DRIFTSORT_HIDDEN driftsort {
template <typename T> T saturating_sub(T a, T b) {
#if __has_builtin(__builtin_sub_overflow)
  T res;
  if (__builtin_sub_overflow(a, b, &res))
    return 0;
  return res;
#else
  return a < b ? 0 : a - b;
#endif
}
} // namespace DRIFTSORT_HIDDEN driftsort
