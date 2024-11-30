/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include "driftsort/common.h"
#include <initializer_list>

namespace driftsort DRIFTSORT_HIDDEN {
namespace small {

inline void sort4_stable(void *raw_base, void *raw_dest,
                         const BlobComparator &comp) {
  BlobPtr base = comp.lift(raw_base);
  BlobPtr dest = comp.lift(raw_dest);
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

/// Merge v assuming v[..len / 2] and v[len / 2..] are sorted.
///
/// Original idea for bi-directional merging by Igor van den Hoven (quadsort),
/// adapted to only use merge up and down. In contrast to the original
/// parity_merge function, it performs 2 writes instead of 4 per iteration.

inline void bidirectional_merge(void *raw_src, size_t len, void *raw_dst,
                                const BlobComparator &comp) {
  struct MergeResult {
    BlobPtr left;
    BlobPtr right;
    BlobPtr dest;

    void merge_up(const BlobComparator &comp) {
      bool advance_left = !comp(right, left);
      BlobPtr src = advance_left ? left : right;
      src.copy_nonoverlapping(dest);
      right = right.offset(!advance_left);
      left = left.offset(advance_left);
      dest = dest.offset(1);
    }

    void merge_down(const BlobComparator &comp) {
      bool retreat_right = !comp(right, left);
      BlobPtr src = retreat_right ? right : left;
      src.copy_nonoverlapping(dest);
      right = right.offset(-retreat_right);
      left = left.offset(-!retreat_right);
      dest = dest.offset(-1);
    }
  };

  BlobPtr src = comp.lift(raw_src);
  BlobPtr dst = comp.lift(raw_dst);

  size_t half = len / 2;
  DRIFTSORT_ASSUME(half > 0);
  MergeResult forward{
      src,
      src.offset(half),
      dst,
  };
  MergeResult backward{src.offset(half - 1), src.offset(len - 1),
                       dst.offset(len - 1)};

  for (size_t i = 0; i < half; i++) {
    forward.merge_up(comp);
    backward.merge_down(comp);
  }

  BlobPtr left_end = backward.left.offset(1);
  BlobPtr right_end = backward.right.offset(1);

  if (len % 2 != 0) {
    bool left_nonempty = forward.left < left_end;
    BlobPtr last_src = left_nonempty ? forward.left : forward.right;
    last_src.copy_nonoverlapping(forward.dest);
    forward.left = forward.left.offset(left_nonempty);
    forward.right = forward.right.offset(!left_nonempty);
  }

  DRIFTSORT_ASSUME(forward.left == left_end);
  DRIFTSORT_ASSUME(forward.right == right_end);
}

inline void sort8_stable(void *raw_base, void *raw_dest, void *raw_scratch,
                         const BlobComparator &comp) {
  BlobPtr base = comp.lift(raw_base);
  BlobPtr dest = comp.lift(raw_dest);
  BlobPtr scratch = comp.lift(raw_scratch);
  sort4_stable(base, scratch, comp);
  sort4_stable(base.offset(4), scratch.offset(4), comp);
  bidirectional_merge(scratch, 8, dest, comp);
}

/// Sorts range [begin, tail] assuming [begin, tail) is already sorted.

inline void insert_tail(void *raw_begin, void *raw_tail,
                        const BlobComparator &comp) {
  class CopyOnDrop {
    BlobPtr src;
    BlobPtr dest;
    size_t length;

  public:
    constexpr CopyOnDrop(BlobPtr src, BlobPtr dest, size_t length)
        : src(src), dest(dest), length(length) {}
    ~CopyOnDrop() { src.copy_nonoverlapping(dest, length); }
    void set_new_dst(BlobPtr new_dest) { dest = new_dest; }
    BlobPtr get_dst() const { return dest; }
  };

  BlobPtr begin = comp.lift(raw_begin);
  BlobPtr tail = comp.lift(raw_tail);
  BlobPtr cursor = tail.offset(-1);

  // fast return if tail is already sorted
  if (!comp(tail, cursor))
    return;

  BlobPtr tmp = DRIFTSORT_ALLOCA(tail.size(), 1);
  tail.copy_nonoverlapping(tmp);
  CopyOnDrop gap_guard(tmp, tail, 1);

  for (;;) {
    cursor.copy_nonoverlapping(gap_guard.get_dst());
    gap_guard.set_new_dst(cursor);
    if (cursor == begin)
      break;
    cursor = cursor.offset(-1);
    if (!comp(tmp, cursor))
      break;
  }
}

/// Sort `v` assuming `v[..offset]` is already sorted.

inline void insertion_sort_shift_left(void *raw_begin, size_t total,
                                      size_t offset,
                                      const BlobComparator &comp) {
  DRIFTSORT_ASSUME(offset > 0 && offset < total);
  BlobPtr begin = comp.lift(raw_begin);
  BlobPtr end = begin.offset(total);
  BlobPtr tail = begin.offset(offset);
  while (tail != end) {
    insert_tail(begin, tail, comp);
    tail = tail.offset(1);
  }
}

// scratch pad is of (16 + length) in size

inline void small_sort_general(void *raw_base, size_t length, void *raw_scratch,
                               const BlobComparator &comp) {
  BlobPtr base = comp.lift(raw_base);
  BlobPtr scratch = comp.lift(raw_scratch);
  if (length < 2)
    return;

  size_t half = length / 2;

  size_t presorted_length;
  if (base.size() <= 16 && length >= 16) {
    // small element and large length
    sort8_stable(base, scratch, scratch.offset(length), comp);
    sort8_stable(base.offset(half), scratch.offset(half),
                 scratch.offset(length + 8), comp);
    presorted_length = 8;
  } else if (length >= 8) {
    sort4_stable(base, scratch, comp);
    sort4_stable(base.offset(half), scratch.offset(half), comp);
    presorted_length = 4;
  } else {
    base.copy_nonoverlapping(scratch);
    base.offset(half).copy_nonoverlapping(scratch.offset(half));
    presorted_length = 1;
  }
  // sort two halves
  for (size_t offset : {size_t{0}, half}) {
    BlobPtr src = base.offset(offset);
    BlobPtr dst = scratch.offset(offset);
    size_t desired_length = offset == 0 ? half : length - half;
    for (size_t i = presorted_length; i < desired_length; i++) {
      src.offset(i).copy_nonoverlapping(dst.offset(i));
      insert_tail(dst, dst.offset(i), comp);
    }
  }
  // merge two halves
  bidirectional_merge(scratch, length, base, comp);
}

} // namespace small
} // namespace driftsort DRIFTSORT_HIDDEN
