/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "driftsort/blob.h"
#include <concepts>
#include <functional>
namespace driftsort {
template <class T> class ElementWithSrc {
  T val;
  T *addr;

public:
  T &operator*() { return val; }
  T *operator->() { return &val; }
  bool operator<(const ElementWithSrc &other) const { return val < other.val; }
  bool operator==(const ElementWithSrc &other) const {
    return val == other.val;
  }
  bool address_less_eq(const ElementWithSrc &other) const {
    return addr <= other.addr;
  }
  void record_address() { addr = &val; }
  ElementWithSrc(T &val) : val(val), addr(nullptr) {}
  ElementWithSrc() = default;
};

template <typename T, std::predicate<const T &, const T &> Comp = std::less<>>
BlobComparator compare_blob() {
  return {
      sizeof(T),
      [](const void *a, const void *b, void *) {
        return Comp{}(*static_cast<const T *>(a), *static_cast<const T *>(b));
      },
      nullptr,
  };
}

} // namespace driftsort
