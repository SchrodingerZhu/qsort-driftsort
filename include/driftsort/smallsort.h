/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include "driftsort/common.h"
#include <concepts>
#include <tuple>

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

using MergeResult = std::tuple<BlobPtr, BlobPtr, BlobPtr>;

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
inline MergeResult merge_up(BlobPtr left, BlobPtr right, BlobPtr dest,
                            Comp &&comp) {
  bool advance_left = !comp(right, left);
  BlobPtr src = advance_left ? left : right;
  src.copy_nonoverlapping(dest);
  right = right.offset(!advance_left);
  left = left.offset(advance_left);
  dest = dest.offset(1);
  return {left, right, dest};
}

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
inline MergeResult merge_down(BlobPtr left, BlobPtr right, BlobPtr dest,
                              Comp &&comp) {
  bool retreat_right = !comp(right, left);
  BlobPtr src = retreat_right ? right : left;
  src.copy_nonoverlapping(dest);
  right = right.offset(-retreat_right);
  left = left.offset(-!retreat_right);
  dest = dest.offset(-1);
  return {left, right, dest};
}

/// Merge v assuming v[..len / 2] and v[len / 2..] are sorted.
///
/// Original idea for bi-directional merging by Igor van den Hoven (quadsort),
/// adapted to only use merge up and down. In contrast to the original
/// parity_merge function, it performs 2 writes instead of 4 per iteration.
template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
void bidirectional_merge(BlobPtr src, size_t len, BlobPtr dst, Comp &&comp) {
  size_t half = len / 2;
  DRIFTSORT_ASSUME(half > 0);
  BlobPtr left = src;
  BlobPtr right = src.offset(half);
  BlobPtr left_rev = src.offset(half - 1);
  BlobPtr right_rev = src.offset(len - 1);
  BlobPtr dst_rev = dst.offset(len - 1);
  for (size_t i = 0; i < half; i++) {
    std::tie(left, right, dst) = merge_up(left, right, dst, comp);
    std::tie(left_rev, right_rev, dst_rev) =
        merge_down(left_rev, right_rev, dst_rev, comp);
  }

  BlobPtr left_end = left_rev.offset(1);
  BlobPtr right_end = right_rev.offset(1);

  if (len % 2 != 0) {
    bool left_nonempty = left < left_end;
    BlobPtr last_src = left_nonempty ? left : right;
    last_src.copy_nonoverlapping(dst);
    left = left.offset(left_nonempty);
    right = right.offset(!left_nonempty);
  }

  DRIFTSORT_ASSUME(left == left_end);
  DRIFTSORT_ASSUME(right == right_end);
}

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
void sort8_stable(BlobPtr base, BlobPtr dest, BlobPtr scratch, Comp &&comp) {
  sort4_stable(base, scratch, comp);
  sort4_stable(base.offset(4), scratch.offset(4), comp);
  bidirectional_merge(scratch, 8, dest, comp);
}

} // namespace small
} // namespace driftsort
