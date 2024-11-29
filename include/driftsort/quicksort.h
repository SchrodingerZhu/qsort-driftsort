/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include "driftsort/common.h"
#include "driftsort/pivot.h"
#include "driftsort/smallsort.h"
#include <concepts>
#include <cstddef>
namespace driftsort {
namespace quick {

struct PartitionState {
  // The current element that is being looked at, scans left to right through
  // slice.
  BlobPtr scan;
  // The start of the scratch auxiliary memory.
  BlobPtr scratch;
  // Counts the number of elements that went to the left side.
  size_t num_left;
  // Reverse scratch output pointer.
  BlobPtr scratch_rev;

  PartitionState(BlobPtr scan, BlobPtr scratch, size_t length)
      : scratch(scratch), scan(scan), num_left(0),
        scratch_rev(scratch.offset(length)) {}

  /// Depending on the value of `towards_left` this function will write a value
  /// to the growing left or right side of the scratch memory. This forms the
  /// branchless core of the partition.
  /// This function may be called at most `len` times. If it is called exactly
  /// `len` times the scratch buffer then contains a copy of each element from
  /// the scan buffer exactly once - a permutation, and num_left <= len.
  BlobPtr partition_once(bool towards_left) {
    scratch_rev = scratch_rev.offset(-1);
    BlobPtr dst = towards_left ? scratch : scratch_rev;
    dst = dst.offset(num_left);
    scan.copy_nonoverlapping(dst);
    num_left += towards_left;
    scan = scan.offset(1);
    return dst;
  }
};

/// Partitions `v` using pivot `p = v[pivot_pos]` and returns the number of
/// elements less than `p`. The relative order of elements that compare < p and
/// those that compare >= p is preserved - it is a stable partition.
///
/// If `is_less` is not a strict total order or panics, `scratch.len() <
/// v.len()`, or `pivot_pos >= v.len()`, the result and `v`'s state is sound but
/// unspecified.

template <typename Comp>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
size_t stable_partition(BlobPtr v, size_t length, BlobPtr scratch,
                        size_t pivot_pos, bool pivot_goes_left, Comp comp) {

  // The core idea is to write the values that compare as less-than to the left
  // side of `scratch`, while the values that compared as greater or equal than
  // `v[pivot_pos]` go to the right side of `scratch` in reverse. See
  // PartitionState for details.
  DRIFTSORT_ASSUME(pivot_pos < length);
  BlobPtr pivot = v.offset(pivot_pos);
  PartitionState state(v, scratch, length);
  BlobPtr pivot_in_scratch;
  size_t loop_end_pos = pivot_pos;
  for (;;) {
    BlobPtr loop_end = v.offset(loop_end_pos);
#ifdef __GNUC__
#pragma unroll(4)
#endif
    while (state.scan < loop_end)
      state.partition_once(comp(state.scan, pivot));

    if (loop_end_pos == length)
      break;

    pivot_in_scratch = state.partition_once(pivot_goes_left);
    loop_end_pos = length;
  }

  scratch.copy_nonoverlapping(v, state.num_left);
  for (size_t i = 0, end = length - state.num_left; i < end; i++)
    scratch.offset(length - 1 - i)
        .copy_nonoverlapping(v.offset(state.num_left + i));

  return state.num_left;
}

/// Sorts `v` recursively using quicksort.
///
/// `limit` when initialized with `c*log(v.len())` for some c ensures we do not
/// overflow the stack or go quadratic.
///
inline constexpr size_t SMALLSORT_THRESHOLD = 32;

template <typename Comp, typename FallBackSort>
  requires std::predicate<Comp, BlobPtr, BlobPtr>
void stable_quicksort(BlobPtr v, size_t length, BlobPtr scratch, size_t limit,
                      BlobPtr left_ancestor_pivot, Comp comp,
                      FallBackSort fallback) {
  for (;;) {
    if (length <= SMALLSORT_THRESHOLD) {
      small::small_sort_general(v, length, scratch, comp);
      return;
    }

    if (limit == 0) {
      fallback(v, length, scratch, comp);
      return;
    }

    limit--;

    size_t pivot_pos = pivot::choose_pivot(v, length, comp);
    DRIFTSORT_ASSUME(pivot_pos < length);

    BlobPtr pivot_copy = DRIFTSORT_ALLOCA(v.size());
    v.offset(pivot_pos).copy_nonoverlapping(pivot_copy);

    // We choose a pivot, and check if this pivot is equal to our left
    // ancestor. If true, we do a partition putting equal elements on the
    // left and do not recurse on it. This gives O(n log k) sorting for k
    // distinct values, a strategy borrowed from pdqsort.
    bool perform_equal_partition = false;
    if (left_ancestor_pivot.get() != nullptr) {
      perform_equal_partition = !comp(left_ancestor_pivot, v.offset(pivot_pos));
    }

    size_t left_partition_len = 0;

    if (!perform_equal_partition) {
      left_partition_len =
          stable_partition(v, length, scratch, pivot_pos, false, comp);
      perform_equal_partition = left_partition_len == 0;
    }

    if (perform_equal_partition) {
      size_t mid_eq =
          stable_partition(v, length, scratch, pivot_pos, true,
                           [&](BlobPtr a, BlobPtr b) { return !comp(b, a); });
      v = v.offset(mid_eq);
      length -= mid_eq;
      left_ancestor_pivot = {};
      continue;
    }

    BlobPtr right = v.offset(left_partition_len);
    size_t right_len = length - left_partition_len;
    stable_quicksort(right, right_len, scratch, limit, pivot_copy, comp,
                     fallback);
    length = left_partition_len;
  }
}

} // namespace quick
} // namespace driftsort