/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm>
#include <concepts>

namespace driftsort {
namespace small {
template <typename T, typename Comp>
  requires std::predicate<Comp, const T &, const T &>
void sort4_stable(const T *__restrict base, T *__restrict dest, Comp comp) {
  bool c1 = comp(base[1], base[0]);
  bool c2 = comp(base[3], base[2]);

  const T *a = &base[c1];
  const T *b = &base[!c1];
  const T *c = &base[2 + c2];
  const T *d = &base[2 + !c2];
  // Compare (a, c) and (b, d) to identify max/min. We're left with two
  // unknown elements, but because we are a stable sort we must know which
  // one is leftmost and which one is rightmost.
  // c3, c4 | min max unknown_left unknown_right
  //  0,  0 |  a   d    b         c
  //  0,  1 |  a   b    c         d
  //  1,  0 |  c   d    a         b
  //  1,  1 |  c   b    a         d
  bool c3 = comp(*c, *a);
  bool c4 = comp(*d, *b);
  const T *min = c3 ? c : a;
  const T *max = c4 ? b : d;
  const T *unknown_left = c3 ? a : (c4 ? c : b);
  const T *unknown_right = c4 ? d : (c3 ? b : c);

  // Sort the last two unknown elements.
  bool c5 = comp(*unknown_right, *unknown_left);
  const T *lo = c5 ? unknown_right : unknown_left;
  const T *hi = c5 ? unknown_left : unknown_right;

  std::copy_n(min, 1, &dest[0]);
  std::copy_n(lo, 1, &dest[1]);
  std::copy_n(hi, 1, &dest[2]);
  std::copy_n(max, 1, &dest[3]);
}
} // namespace small
} // namespace driftsort
