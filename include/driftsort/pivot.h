/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include <concepts>
#include <cstddef>

namespace driftsort {
namespace pivot {
// Recursively select a pseudomedian if above this threshold.
inline constexpr size_t PSEUDO_MEDIAN_REC_THRESHOLD = 64;

/// Calculates the median of 3 elements.
///
/// SAFETY: a, b, c must be valid initialized elements.

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
BlobPtr median_of_3(BlobPtr a, BlobPtr b, BlobPtr c, Comp comp) {
  // Compiler tends to make this branchless when sensible, and avoids the
  // third comparison when not.
  bool x = comp(a, b);
  bool y = comp(a, c);
  if (x == y) {
    // If x=y=0 then b, c <= a. In this case we want to return max(b, c).
    // If x=y=1 then a < b, c. In this case we want to return min(b, c).
    // By toggling the outcome of b < c using XOR x we get this behavior.
    bool z = comp(b, c);
    return z ^ x ? c : b;
  }
  // Either c <= a < b or b <= a < c, thus a is our median.
  return a;
}

/// Calculates an approximate median of 3 elements from sections a, b, c, or
/// recursively from an approximation of each, if they're large enough. By
/// dividing the size of each section by 8 when recursing we have logarithmic
/// recursion depth and overall sample from f(n) = 3*f(n/8) -> f(n) =
/// O(n^(log(3)/log(8))) ~= O(n^0.528) elements.
///
/// SAFETY: a, b, c must point to the start of initialized regions of memory of
/// at least n elements
template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
BlobPtr recursive_median_of_3(BlobPtr a, BlobPtr b, BlobPtr c, size_t n,
                              Comp comp) {
  if (n * 8 >= PSEUDO_MEDIAN_REC_THRESHOLD) {
    size_t n8 = n / 8;
    a = recursive_median_of_3(a, a.offset(n8 * 4), a.offset(n8 * 7), n8, comp);
    b = recursive_median_of_3(b, b.offset(n8 * 4), b.offset(n8 * 7), n8, comp);
    c = recursive_median_of_3(c, c.offset(n8 * 4), c.offset(n8 * 7), n8, comp);
  }
  return median_of_3(a, b, c, comp);
}

/// Selects a pivot from `v`. Algorithm taken from glidesort by Orson Peters.
///
/// This chooses a pivot by sampling an adaptive amount of points, approximating
/// the quality of a median of sqrt(n) elements.

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
size_t choose_pivot(BlobPtr v, size_t length, Comp comp) {
  size_t length_div_8 = length / 8;
  BlobPtr a = v;
  BlobPtr b = v.offset(length_div_8 * 4);
  BlobPtr c = v.offset(length_div_8 * 7);

  if (length < PSEUDO_MEDIAN_REC_THRESHOLD)
    return static_cast<size_t>(median_of_3(a, b, c, comp) - v);

  return static_cast<size_t>(
      recursive_median_of_3(a, b, c, length_div_8, comp) - v);
}

} // namespace pivot
} // namespace driftsort
