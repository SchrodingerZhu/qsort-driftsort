/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include "driftsort/common.h"
#include "driftsort/merge.h"
#include "driftsort/quicksort.h"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace DRIFTSORT_HIDDEN driftsort {
namespace drift {
template <typename Comp>
inline void stable_quicksort(void *raw_v, size_t length, void *raw_scratch,
                             size_t scratch_length,
                             const BlobComparator<Comp> &comp) {
  size_t limit = std::bit_width(2 * (length | 1));
  quick::stable_quicksort(raw_v, length, raw_scratch, scratch_length, limit,
                          nullptr, comp);
}
class RunState {
  size_t state;

  constexpr RunState(size_t state) : state(state) {}

public:
  constexpr RunState() = default;
  constexpr static RunState sorted(size_t length) {
    return RunState((length << size_t{1}) | size_t{1});
  }
  constexpr static RunState unsorted(size_t length) {
    return RunState(length << size_t{1});
  }
  bool is_sorted() const { return state & size_t{1}; }
  size_t length() const { return state >> size_t{1}; }
};

/// Finds a run of sorted elements starting at the beginning of the slice.
///
/// Returns the length of the run, and a bool that is false when the run
/// is ascending, and true if the run strictly descending.
template <typename Comp>
inline bool find_existing_run(void *raw_v, size_t length,
                              const BlobComparator<Comp> &comp, size_t &out) {
  if (length < 2) {
    out = length;
    return false;
  }

  BlobPtr v = comp.lift(raw_v);
  size_t run_length = 2;
  bool strictly_descending = comp(v.offset(1), v.offset(0));
  if (strictly_descending) {
    while (run_length < length &&
           comp(v.offset(run_length), v.offset(run_length - 1)))
      run_length++;
  } else {
    while (run_length < length &&
           !comp(v.offset(run_length), v.offset(run_length - 1)))
      run_length++;
  }
  out = run_length;
  return strictly_descending;
}

/// Creates a new logical run.
///
/// A logical run can either be sorted or unsorted. If there is a pre-existing
/// run that clears the `min_good_run_len` threshold it is returned as a sorted
/// run. If not, the result depends on the value of `eager_sort`. If it is true,
/// then a sorted run of length `SMALL_SORT_THRESHOLD` is returned, and if it
/// is false an unsorted run of length `min_good_run_len` is returned.
template <bool eager_sort, typename Comp>
inline RunState create_run(void *raw_v, size_t length, void *raw_scratch,
                           size_t scratch_length, size_t min_good_run_length,
                           const BlobComparator<Comp> &comp) {
  auto v = comp.lift(raw_v);
  auto alloca_space = DRIFTSORT_ALLOCA(comp, 1);
  BlobPtr tmp = comp.lift_alloca(alloca_space);
  auto reverse = [&](BlobPtr array, size_t length) {
    for (size_t i = 0; i < length / 2; i++) {
      auto a = array.offset(i);
      auto b = array.offset(length - 1 - i);
      a.copy_nonoverlapping(tmp);
      b.copy_nonoverlapping(a);
      tmp.copy_nonoverlapping(b);
    }
  };
  if (length >= min_good_run_length) {
    size_t run_length;
    bool descending = find_existing_run(raw_v, length, comp, run_length);
    DRIFTSORT_ASSUME(run_length <= length);
    if (run_length >= min_good_run_length) {
      if (descending)
        reverse(v, run_length);
      return RunState::sorted(run_length);
    }
  }

  if constexpr (eager_sort) {
    size_t eager_sort_len = std::min(length, quick::SMALLSORT_THRESHOLD);
    small::small_sort_general(v, eager_sort_len, raw_scratch, comp);
    return RunState::sorted(eager_sort_len);
  }

  return RunState::unsorted(std::min(length, min_good_run_length));
}

// Lazy logical runs as in Glidesort.
template <typename Comp>
inline RunState logical_merge(void *raw_v, size_t length, void *raw_scratch,
                              size_t scratch_length, RunState left,
                              RunState right,
                              const BlobComparator<Comp> &comp) {
  BlobPtr v = comp.lift(raw_v);
  bool fit_in_scratch = length <= scratch_length;
  if (!fit_in_scratch || left.is_sorted() || right.is_sorted()) {
    if (!left.is_sorted())
      stable_quicksort(v, left.length(), raw_scratch, scratch_length, comp);
    if (!right.is_sorted())
      stable_quicksort(v.offset(left.length()), length - left.length(),
                       raw_scratch, scratch_length, comp);
    merge::merge(v, length, raw_scratch, scratch_length, left.length(), comp);
    return RunState::sorted(length);
  }
  return RunState::unsorted(length);
}

inline size_t approximate_sqrt(size_t n) {
  size_t ilog = std::bit_width(n | size_t{1});
  size_t shift = (size_t{1} + ilog) / size_t{2};
  return ((size_t{1} << shift) + (n >> shift)) / size_t{2};
}

inline uint8_t merge_tree_depth(size_t left, size_t mid, size_t right,
                                uint64_t scale_factor) {
  uint64_t x = static_cast<uint64_t>(left) + static_cast<uint64_t>(mid);
  uint64_t y = static_cast<uint64_t>(mid) + static_cast<uint64_t>(right);
  return static_cast<uint8_t>(
      std::countl_zero((scale_factor * x) ^ (scale_factor * y)));
}

// Nearly-Optimal Mergesorts: Fast, Practical Sorting Methods That Optimally
// Adapt to Existing Runs by J. Ian Munro and Sebastian Wild.
//
// This method forms a binary merge tree, where each internal node corresponds
// to a splitting point between the adjacent runs that have to be merged. If we
// visualize our array as the number line from 0 to 1, we want to find the
// dyadic fraction with smallest denominator that lies between the midpoints of
// our to-be-merged slices. The exponent in the dyadic fraction indicates the
// desired depth in the binary merge tree this internal node wishes to have.
// This does not always correspond to the actual depth due to the inherent
// imbalance in runs, but we follow it as closely as possible.
//
// As an optimization we rescale the number line from [0, 1) to [0, 2^62). Then
// finding the simplest dyadic fraction between midpoints corresponds to finding
// the most significant bit difference of the midpoints. We save scale_factor =
// ceil(2^62 / n) to perform this rescaling using a multiplication, avoiding
// having to repeatedly do integer divides. This rescaling isn't exact when n is
// not a power of two since we use integers and not reals, but the result is
// very close, and in fact when n < 2^30 the resulting tree is equivalent as the
// approximation errors stay entirely in the lower order bits.
//
// Thus for the splitting point between two adjacent slices [a, b) and [b, c)
// the desired depth of the corresponding merge node is CLZ((a+b)*f ^ (b+c)*f),
// where CLZ counts the number of leading zeros in an integer and f is our scale
// factor. Note that we omitted the division by two in the midpoint
// calculations, as this simply shifts the bits by one position (and thus always
// adds one to the result), and we only care about the relative depths.
//
// Finally, if we try to upper bound x = (a+b)*f giving x = (n-1 + n) *
// ceil(2^62 / n) then
//    x < (2^62 / n + 1) * 2n
//    x < 2^63 + 2n
// So as long as n < 2^62 we find that x < 2^64, meaning our operations do not
// overflow.

inline uint64_t merge_tree_scale_factor(size_t n) {
  static_assert(sizeof(size_t) <= sizeof(uint64_t));
  uint64_t n64 = static_cast<uint64_t>(n);
  return ((uint64_t{1} << uint64_t{62}) + n64 - uint64_t{1}) / n64;
}
template <bool eager_sort, typename Comp>
inline void sort(void *raw_v, size_t length, void *raw_scratch,
                 size_t scratch_length, const BlobComparator<Comp> &comp) {
  if (length < 2)
    return;
  size_t scale_factor = merge_tree_scale_factor(length);

  // It's important to have a relatively high entry barrier for pre-sorted
  // runs, as the presence of a single such run will force on average several
  // merge operations and shrink the maximum quicksort size a lot. For that
  // reason we use sqrt(len) as our pre-sorted run threshold.
  constexpr size_t MIN_SQRT_RUN_LEN = 64;
  size_t min_good_run_len =
      (length <= MIN_SQRT_RUN_LEN * MIN_SQRT_RUN_LEN)
          ? std::min(length - length / 2, MIN_SQRT_RUN_LEN)
          : approximate_sqrt(length);

  size_t stack_length = 0;
  DRIFTSORT_UNINITIALIZED RunState run_storage[66];
  DRIFTSORT_UNINITIALIZED uint8_t depth_storage[66];

  size_t scan_idx = 0;
  RunState prev_run = RunState::sorted(0);

  BlobPtr v = comp.lift(raw_v);
  BlobPtr scratch = comp.lift(raw_scratch);

  for (;;) {
    RunState next_run;
    uint8_t desired_depth;
    if (scan_idx < length) {
      next_run =
          create_run<eager_sort>(v.offset(scan_idx), length - scan_idx, scratch,
                                 scratch_length, min_good_run_len, comp);
      desired_depth =
          merge_tree_depth(scan_idx - prev_run.length(), scan_idx,
                           scan_idx + next_run.length(), scale_factor);
    } else {
      next_run = RunState::sorted(0);
      desired_depth = 0;
    }

    // Process the merge nodes between earlier runs[i] that have a desire to
    // be deeper in the merge tree than the merge node for the splitpoint
    // between prev_run and next_run.
    //
    // SAFETY: first note that this is the only place we modify stack_len,
    // runs or desired depths. We maintain the following invariants:
    //  1. The first stack_len elements of runs/desired_depths are initialized.
    //  2. For all valid i > 0, desired_depths[i] < desired_depths[i+1].
    //  3. The sum of all valid runs[i].len() plus prev_run.len() equals
    //     scan_idx.

    while (stack_length > 1 &&
           depth_storage[stack_length - 1] >= desired_depth) {
      RunState left = run_storage[stack_length - 1];
      size_t merge_length = left.length() + prev_run.length();
      size_t merge_start_index = scan_idx - merge_length;
      BlobPtr merge_start = v.offset(merge_start_index);
      prev_run = logical_merge(merge_start, merge_length, scratch,
                               scratch_length, left, prev_run, comp);
      stack_length--;
    }

    // We now know that desired_depths[stack_len - 1] < desired_depth,
    // maintaining our invariant. This also guarantees we don't overflow
    // the stack as merge_tree_depth(..) <= 64 and thus we can only have
    // 64 distinct values on the stack before pushing, plus our initial
    // dummy run, while our capacity is 66.

    run_storage[stack_length] = prev_run;
    depth_storage[stack_length] = desired_depth;
    stack_length++;

    if (scan_idx >= length)
      break;

    scan_idx += next_run.length();
    prev_run = next_run;
  }

  if (!prev_run.is_sorted())
    stable_quicksort(v, length, scratch, scratch_length, comp);
}

} // namespace drift
} // namespace DRIFTSORT_HIDDEN driftsort
