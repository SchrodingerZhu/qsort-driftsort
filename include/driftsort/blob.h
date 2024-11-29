/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

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
  void copy_nonoverlapping(BlobPtr dest) const {
    std::memcpy(dest, data, element_size);
  }
  void *get() const { return data; }
  constexpr operator void *() const { return data; }
  constexpr BlobPtr offset(ptrdiff_t offset) const {
    return {element_size, data + offset * static_cast<ptrdiff_t>(element_size)};
  }
};
} // namespace driftsort