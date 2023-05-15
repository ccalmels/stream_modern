#include <stream.hpp>

std::vector<u8> Stream::extract(size_t n) {
  std::vector<u8> ret = std::vector<u8>(n, 0);

  s_.read(ret.data(), n);

  ret.resize(s_.gcount());

  return ret;
}

void Stream::extract_into(std::vector<u8> &v, size_t n) {
  size_t orig_size = v.size();

  v.resize(orig_size + n);

  s_.read(v.data() + orig_size, n);

  v.resize(orig_size + s_.gcount());
}

Stream &Stream::operator<<(const std::string_view &sv) {
  s_ << (u8)sv.size();

  std::for_each(sv.begin(), sv.end(), [this](const auto e) { s_ << (u8)e; });

  return *this;
}

Stream &Stream::operator<<(const std::string &s) {
  return (*this) << (std::string_view)s;
}

Stream &Stream::operator>>(std::string &s) {
  u8 size, element;

  (*this) >> size;

  for (u8 i = 0; i < size; i++) {
    if ((*this) >> element)
      s.push_back((char)element);
  }

  return *this;
}

std::ostream &operator<<(std::ostream &os, const Stream &st) {
  auto flags = os.setf(std::ios::hex, std::ios::basefield);

  os << std::setfill('0');

  std::ranges::for_each(st.s_.view(),
                        [&os](const u8 e) { os << std::setw(2) << (int)e; });

  os.setf(flags);
  return os;
}
