#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <ranges>
#include <vector>

template <std::integral T> constexpr T byteswap(T value) noexcept {
#if __cpp_lib_byteswap < 202110L
    // snippet comes from cppreference.com
    static_assert(std::has_unique_object_representations_v<T>,
                  "T may not have padding bits");

    auto value_representation =
        std::bit_cast<std::array<std::byte, sizeof(T)>>(value);

    std::ranges::reverse(value_representation);

    return std::bit_cast<T>(value_representation);
#else
    return std::byteswap<T>(value);
#endif
}
using u8 = uint8_t;

class Stream {
  public:
    template <std::integral T> Stream &operator<<(const T &value) {
        T v = value;
        if constexpr (std::endian::native == std::endian::big)
            v = byteswap<T>(v);
        auto bytes = std::bit_cast<std::array<u8, sizeof(T)>>(v);
        write_(bytes.data(), bytes.size());
        return *this;
    }

    template <std::integral T> Stream &operator>>(T &value) {
        std::array<u8, sizeof(T)> bytes;
        read_(bytes.data(), bytes.size());
        value = std::bit_cast<T>(bytes);
        if constexpr (std::endian::native == std::endian::big)
            value = byteswap<T>(value);
        return *this;
    }

    std::vector<u8> extract(size_t n);
    void extract_into(std::vector<u8> &v, size_t n);

    std::vector<u8> extract() { return extract(remaining()); }
    void extract_into(std::vector<u8> &v) { extract_into(v, remaining()); }

    Stream &operator<<(const std::ranges::contiguous_range auto &range) {
        write_(std::ranges::cdata(range), std::ranges::size(range));
        return *this;
    }
    Stream &operator<<(std::string_view sv);
    Stream &operator<<(const std::string &s);
    Stream &operator>>(std::string &s);

    explicit operator bool() const { return !fail_; }

    void swap(Stream &other) noexcept {
        buf_.swap(other.buf_);
        std::swap(read_pos_, other.read_pos_);
        std::swap(fail_, other.fail_);
    }

    friend std::ostream &operator<<(std::ostream &os, const Stream &st);

  private:
    void write_(const u8 *ptr, size_t n) {
        buf_.insert(buf_.end(), ptr, ptr + n);
    }

    size_t read_(u8 *ptr, size_t n) {
        size_t available = buf_.size() - read_pos_;
        size_t to_read = std::min(n, available);
        std::copy_n(buf_.data() + read_pos_, to_read, ptr);
        read_pos_ += to_read;
        if (to_read < n)
            fail_ = true;
        return to_read;
    }

    size_t remaining() const { return buf_.size() - read_pos_; }

    std::vector<u8> buf_;
    size_t read_pos_ = 0;
    bool fail_ = false;
};
