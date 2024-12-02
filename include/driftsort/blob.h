/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/common.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

namespace DRIFTSORT_HIDDEN driftsort {

// A fat pointer with size
class BlobPtr {
  size_t element_size;
  std::byte *data;

public:
  template <class T>
    requires std::is_trivial_v<T>
  constexpr BlobPtr(T *data)
      : element_size(sizeof(T)), data(reinterpret_cast<std::byte *>(data)) {}
  constexpr BlobPtr() : element_size(0), data(nullptr) {}
  constexpr BlobPtr(size_t element_size, std::byte *data)
      : element_size(element_size), data(data) {}
  void copy_nonoverlapping(BlobPtr dst, size_t n = 1) const {
#if __has_builtin(__builtin_memcpy_inline)
    if (element_size <= 16 && n == 1) {
      std::byte *src = data;
      std::byte *dest = dst.data;
      if (element_size >= 8) {
        __builtin_memcpy_inline(dest, src, 8);
        __builtin_memcpy_inline(&dest[element_size - 8], &src[element_size - 8],
                                8);
      } else if (element_size >= 4) {
        __builtin_memcpy_inline(dest, src, 4);
        __builtin_memcpy_inline(&dest[element_size - 4], &src[element_size - 4],
                                4);
      } else if (element_size >= 2) {
        __builtin_memcpy_inline(dest, src, 2);
        __builtin_memcpy_inline(&dest[element_size - 2], &src[element_size - 2],
                                2);
      } else {
        __builtin_memcpy_inline(dest, src, 1);
      }
      return;
    }
#endif
    std::memcpy(dst, data, element_size * n);
  }
  void *get() const { return data; }
  size_t size() const { return element_size; }
  constexpr operator void *() const { return data; }
  constexpr BlobPtr offset(ptrdiff_t offset) const {
    return {element_size, data + offset * static_cast<ptrdiff_t>(element_size)};
  }
  constexpr bool operator<(BlobPtr other) const { return data < other.data; }
  DRIFTSORT_CONST constexpr bool operator==(BlobPtr other) const {
    return data == other.data;
  }
  ptrdiff_t operator-(BlobPtr other) const {
    return (data - other.data) / static_cast<ptrdiff_t>(element_size);
  }
};

template <class Comparator> class BlobComparator {
  size_t element_size;
  size_t alignment;
  Comparator compare;

public:
  constexpr BlobComparator(size_t element_size, size_t alignment,
                           Comparator compare)
      : element_size(element_size), alignment(alignment), compare(compare) {}
  constexpr BlobPtr lift(void *data) const {
    return {element_size, static_cast<std::byte *>(data)};
  }
  bool operator()(const void *a, const void *b) const {
    return compare(a, b) < 0;
  }
  size_t size() const { return element_size; }
  size_t align() const { return alignment; }
  size_t alloca_padding() const {
    return saturating_sub(alignment, alignof(std::max_align_t));
  }
  constexpr BlobPtr lift_alloca(std::byte *data) const {
    uintptr_t addr = reinterpret_cast<uintptr_t>(data);
    addr += (-addr) & (alignment - 1);
    return {element_size, reinterpret_cast<std::byte *>(addr)};
  }
};

} // namespace DRIFTSORT_HIDDEN driftsort
