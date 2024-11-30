/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "driftsort/blob.h"
namespace driftsort DRIFTSORT_HIDDEN {
namespace merge {
/// Merges non-decreasing runs `v[..mid]` and `v[mid..]` using `scratch` as
/// temporary storage, and stores the result into `v[..]`.

template <typename Comp>
inline void merge(void *raw_v, size_t length, void *raw_scratch,
                  size_t scratch_length, size_t mid,
                  const BlobComparator<Comp> &comp) {
  struct MergeState {
    BlobPtr start;
    BlobPtr end;
    BlobPtr dest;

    ~MergeState() {
      size_t length = end - start;
      start.copy_nonoverlapping(dest, length);
    }

    void merge_up(BlobPtr right, BlobPtr right_end,
                  const BlobComparator<Comp> &comp) {
      BlobPtr &left = start;
      BlobPtr &out = dest;

      while (left != end && right != right_end) {
        bool consume_left = !comp(right, left);
        BlobPtr src = consume_left ? left : right;
        src.copy_nonoverlapping(out);
        left = left.offset(consume_left);
        right = right.offset(!consume_left);
        out = out.offset(1);
      }
    }

    void merge_down(BlobPtr left_end, BlobPtr right_end, BlobPtr out,
                    const BlobComparator<Comp> &comp) {
      do {
        BlobPtr left = dest.offset(-1);
        BlobPtr right = end.offset(-1);
        out = out.offset(-1);

        bool consume_left = comp(right, left);
        BlobPtr src = consume_left ? left : right;
        src.copy_nonoverlapping(out);
        dest = left.offset(!consume_left);
        end = right.offset(consume_left);
      } while (dest != left_end && end != right_end);
    }
  };
  if (mid == 0 || mid >= length || scratch_length < mid ||
      scratch_length < length - mid)
    return;

  BlobPtr v = comp.lift(raw_v);
  BlobPtr scratch = comp.lift(raw_scratch);
  BlobPtr v_mid = v.offset(mid);
  BlobPtr v_end = v.offset(length);

  size_t left_len = mid;
  size_t right_len = length - mid;
  bool left_is_shorter = left_len <= right_len;
  BlobPtr save_base = left_is_shorter ? v : v_mid;
  size_t save_len = left_is_shorter ? left_len : right_len;
  save_base.copy_nonoverlapping(scratch, save_len);

  MergeState state{scratch, scratch.offset(save_len), save_base};

  if (left_is_shorter) {
    state.merge_up(v_mid, v_end, comp);
  } else {
    state.merge_down(v, scratch, v_end, comp);
  }
}
} // namespace merge
} // namespace driftsort DRIFTSORT_HIDDEN
