/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#if __has_builtin(__assume)
#define DRIFTSORT_ASSUME(x) __assume(x)
#elif __has_builtin(__builtin_assume)
#define DRIFTSORT_ASSUME(x) __builtin_assume(x)
#elif __has_builtin(__builtin_unreachable)
#define DRIFTSORT_ASSUME(x)                                                    \
  do {                                                                         \
    if (!(x))                                                                  \
      __builtin_unreachable();                                                 \
  } while (0)
#else
#define DRIFTSORT_ASSUME(x) (void)(x)
#endif

#ifdef __GNUC__
#define DIRFTSORT_LIKELY(x) __builtin_expect(!!(x), 1)
#define DIRFTSORT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define DRIFTSORT_CONST [[gnu::const]]
#else
#define DIRFTSORT_LIKELY(x) (x)
#define DIRFTSORT_UNLIKELY(x) (x)
#define DRIFTSORT_CONST
#endif

#ifdef __GNUC__
#define DRIFTSORT_ALLOCA(n)                                                    \
  (BlobPtr{n, static_cast<std::byte *>(__builtin_alloca(n))})
#else
#define DRIFTSORT_ALLOCA(n) (BlobPtr{n, static_cast<std::byte *>(_malloca(n))})
#endif
