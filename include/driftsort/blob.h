/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driftsort/common.h"
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace driftsort {

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
  void copy_nonoverlapping(BlobPtr dest, size_t n = 1) const {
    std::memcpy(dest, data, element_size * n);
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

using Comparator = bool (*)(const void *, const void *, void *);

class BlobComparator {
  size_t element_size;
  Comparator compare;
  void *context;

public:
  constexpr BlobComparator(size_t element_size, Comparator compare,
                           void *context)
      : element_size(element_size), compare(compare), context(context) {}
  constexpr BlobPtr lift(void *data) const {
    return {element_size, static_cast<std::byte *>(data)};
  }
  bool operator()(const void *a, const void *b) const {
    return compare(a, b, context);
  }
  auto transform(Comparator transform) const {
    return BlobComparator{element_size, transform,
                          const_cast<void *>(static_cast<const void *>(this))};
  }
};

} // namespace driftsort
