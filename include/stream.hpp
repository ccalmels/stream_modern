#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <vector>

#if __cpp_lib_byteswap < 202110L
// snippet comes from cppreference.com
template <std::integral T> constexpr T byteswap(T value) noexcept {
  static_assert(std::has_unique_object_representations_v<T>,
                "T may not have padding bits");

  auto value_representation =
      std::bit_cast<std::array<std::byte, sizeof(T)>>(value);

  std::ranges::reverse(value_representation);

  return std::bit_cast<T>(value_representation);
}
#else
template <typename T> const auto byteswap = std::byteswap<T>;
#endif

using u8 = uint8_t;

class Stream {
public:
  template <std::integral T> Stream &operator<<(const T &value) {
    if constexpr (std::endian::native == std::endian::big) {
      T swapped = byteswap<T>(value);

      s_.write((const u8 *)&swapped, sizeof(swapped));
    } else
      s_.write((const u8 *)&value, sizeof(value));
    return *this;
  }

  template <std::integral T> Stream &operator>>(T &value) {
    if constexpr (std::endian::native == std::endian::big) {
      T swapped;

      s_.read((u8 *)&swapped, sizeof(swapped));

      value = byteswap<T>(swapped);
    } else
      s_.read((u8 *)&value, sizeof(value));

    return *this;
  }

  std::vector<u8> extract(size_t n);
  void extract_into(std::vector<u8> &v, size_t n);

  std::vector<u8> extract() { return extract(s_.view().size()); }
  void extract_into(std::vector<u8> &v) { extract_into(v, s_.view().size()); }

  Stream &operator<<(const std::ranges::contiguous_range auto &range) {
    s_.write(std::ranges::cdata(range), std::ranges::size(range));
    return *this;
  }
  Stream &operator<<(const std::string &s);
  Stream &operator>>(std::string &s);

  explicit operator bool() const { return !!s_; };
  bool operator!() const { return !s_; };

  friend std::ostream &operator<<(std::ostream &os, const Stream &st);

private:
  std::basic_stringstream<u8> s_;
};
