/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/blob.h"
#include "driftsort/common.h"
#include "driftsort/drift.h"
#include "driftsort/quicksort.h"
#include "driftsort/smallsort.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace DRIFTSORT_HIDDEN driftsort {
template <typename Comp>
DRIFTSORT_NOINLINE inline void
trivial_heap_sort(void *raw_v, size_t length,
                  const BlobComparator<Comp> &comp) {
  BlobPtr array = comp.lift(raw_v);
  size_t end = length;
  size_t start = end / 2;
  auto left_child = [](size_t i) -> size_t { return 2 * i + 1; };
  auto alloca_space = DRIFTSORT_ALLOCA(comp, 1);
  BlobPtr tmp = comp.lift(alloca_space);
  auto swap = [&](BlobPtr a, BlobPtr b) {
    a.copy_nonoverlapping(tmp);
    b.copy_nonoverlapping(a);
    tmp.copy_nonoverlapping(b);
  };
  while (end > 1) {
    if (start > 0) {
      // Select the next unheapified element to sift down.
      --start;
    } else {
      // Extract the max element of the heap, moving a leaf to root to be sifted
      // down.
      --end;
      swap(array.offset(0), array.offset(end));
    }

    // Sift start down the heap.
    size_t root = start;
    while (left_child(root) < end) {
      size_t child = left_child(root);
      // If there are two children, set child to the greater.
      if (child + 1 < end && comp(array.offset(child), array.offset(child + 1)))
        ++child;

      // If the root is less than the greater child
      if (!comp(array.offset(root), array.offset(child)))
        break;

      // Swap the root with the greater child and continue sifting down.
      swap(array.offset(root), array.offset(child));
      root = child;
    }
  }
}

inline size_t guess_alignment(size_t element_size, void *start_addr) {
  uintptr_t addr = reinterpret_cast<uintptr_t>(start_addr);
  uintptr_t masked = element_size | addr;
  return static_cast<size_t>(masked & -masked);
}

template <typename Comp>
DRIFTSORT_NOINLINE inline void driftsort(void *raw_v, size_t length,
                                         const BlobComparator<Comp> &comp) {
  BlobPtr v = comp.lift(raw_v);
  constexpr size_t MAX_FULL_ALLOC_BYTES = 8 * 1024 * 1024;
  constexpr size_t HEAP_ALLOC_THRESHOLD = 4096;
  bool eager_sort = length <= quick::SMALLSORT_THRESHOLD * 2;
  size_t max_full_alloc = MAX_FULL_ALLOC_BYTES / v.size();
  size_t alloc_length = std::max(length / 2, std::min(length, max_full_alloc));
  alloc_length = std::max(alloc_length, quick::SMALLSORT_THRESHOLD + 16);
  if (alloc_length > HEAP_ALLOC_THRESHOLD) {
    auto raw_scratch =
        ::operator new(alloc_length * v.size(), std::align_val_t{comp.align()});
    if (DRIFTSORT_UNLIKELY(raw_scratch == nullptr))
      return trivial_heap_sort(raw_v, length, comp);
    BlobPtr scratch{v.size(), static_cast<std::byte *>(raw_scratch)};
    if (eager_sort)
      drift::sort<true>(v, length, scratch, alloc_length, comp);
    else
      drift::sort<false>(v, length, scratch, alloc_length, comp);
    ::operator delete(raw_scratch, std::align_val_t{comp.align()});
  } else {
    auto raw_scratch_space = DRIFTSORT_ALLOCA(comp, alloc_length);
    BlobPtr scratch = comp.lift_alloca(raw_scratch_space);
    if (eager_sort)
      drift::sort<true>(v, length, scratch, alloc_length, comp);
    else
      drift::sort<false>(v, length, scratch, alloc_length, comp);
  }
}
template <typename Comp>
inline void qsort_r(void *data, size_t length, size_t element_size,
                    Comp compare) {
  if (element_size == 0)
    return;
  if (DRIFTSORT_LIKELY(length < 2))
    return;

  size_t alignment = guess_alignment(element_size, data);
  BlobComparator<Comp> comp{element_size, alignment, compare};

  // Insertion sort is always fine because we never call comparison on temporary
  // space
  constexpr size_t MAX_LEN_ALWAYS_INSERTION_SORT = 20;
  constexpr size_t MAX_ALIGNMENT = 32;
  if (DRIFTSORT_LIKELY(length <= MAX_LEN_ALWAYS_INSERTION_SORT)) {
    BlobPtr v = comp.lift(data);
    return small::insertion_sort_shift_left(v, length, 1, comp);
  }

  // unfortunately, due to qsort_r's restriction, we cannot really know the
  // alignment of over-aligned types, so we have to fall back to slow path.
  // This is similar to the behavior of glibc's qsort.
  if (DRIFTSORT_UNLIKELY(alignment > MAX_ALIGNMENT))
    return trivial_heap_sort(data, length, comp);

  driftsort(data, length, comp);
}
} // namespace DRIFTSORT_HIDDEN driftsort
