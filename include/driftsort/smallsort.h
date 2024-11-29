/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include <concepts>

namespace driftsort {
namespace small {
template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
inline void sort4_stable(BlobPtr base, BlobPtr dest, Comp &&comp) {
  bool c1 = comp(base.offset(1), base.offset(0));
  bool c2 = comp(base.offset(3), base.offset(2));

  BlobPtr a = base.offset(c1);
  BlobPtr b = base.offset(!c1);
  BlobPtr c = base.offset(2 + c2);
  BlobPtr d = base.offset(2 + !c2);
  // Compare (a, c) and (b, d) to identify max/min. We're left with two
  // unknown elements, but because we are a stable sort we must know which
  // one is leftmost and which one is rightmost.
  // c3, c4 | min max unknown_left unknown_right
  //  0,  0 |  a   d    b         c
  //  0,  1 |  a   b    c         d
  //  1,  0 |  c   d    a         b
  //  1,  1 |  c   b    a         d
  bool c3 = comp(c, a);
  bool c4 = comp(d, b);
  BlobPtr min = c3 ? c : a;
  BlobPtr max = c4 ? b : d;
  BlobPtr unknown_left = c3 ? a : (c4 ? c : b);
  BlobPtr unknown_right = c4 ? d : (c3 ? b : c);

  // Sort the last two unknown elements.
  bool c5 = comp(unknown_right, unknown_left);
  BlobPtr lo = c5 ? unknown_right : unknown_left;
  BlobPtr hi = c5 ? unknown_left : unknown_right;

  min.copy_nonoverlapping(dest.offset(0));
  lo.copy_nonoverlapping(dest.offset(1));
  hi.copy_nonoverlapping(dest.offset(2));
  max.copy_nonoverlapping(dest.offset(3));
}
} // namespace small
} // namespace driftsort
