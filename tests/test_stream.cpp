#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stream.hpp>

class StreamFixture : public ::testing::Test {
protected:
};

TEST_F(StreamFixture, Extract) {
  Stream st;
  st << (int)0xdeadbeef;
  st << (short)0xafac;
  st << std::string("foo");
  st << std::string("barbar");

  EXPECT_TRUE(st);
  EXPECT_EQ(st.extract(sizeof(int)), std::vector<u8>({0xef, 0xbe, 0xad, 0xde}));
  EXPECT_EQ(st.extract(sizeof(short)), std::vector<u8>({0xac, 0xaf}));
  EXPECT_EQ(st.extract(4), std::vector<u8>({3, 'f', 'o', 'o'}));
  EXPECT_EQ(st.extract(7), std::vector<u8>({6, 'b', 'a', 'r', 'b', 'a', 'r'}));
}

TEST_F(StreamFixture, ExtractInto) {
  Stream st;
  std::vector<u8> a, b = {'a', 'b'};
  std::vector<u8> as, bs = {'a', 's', 'b', 's'};

  st << (int)0xdeadbeef;
  st << (short)0xafac;
  st << std::string("foo");
  st << std::string("barbar");

  EXPECT_TRUE(st);

  st.extract_into(a, sizeof(int));
  st.extract_into(b, sizeof(short));
  st.extract_into(as, 4);
  st.extract_into(bs, 7);

  EXPECT_EQ(a, std::vector<u8>({0xef, 0xbe, 0xad, 0xde}));
  EXPECT_EQ(b, std::vector<u8>({'a', 'b', 0xac, 0xaf}));
  EXPECT_EQ(as, std::vector<u8>({3, 'f', 'o', 'o'}));
  EXPECT_EQ(bs, std::vector<u8>(
                    {'a', 's', 'b', 's', 6, 'b', 'a', 'r', 'b', 'a', 'r'}));
}

TEST_F(StreamFixture, ExtractAll) {
  Stream st;

  st << (int)0xdeadbeef;
  st << (short)0xafac;
  st << std::string("foo");
  st << std::string("barbar");

  EXPECT_TRUE(st);
  EXPECT_EQ(st.extract(), std::vector<u8>({
                              0xef,
                              0xbe,
                              0xad,
                              0xde,
                              0xac,
                              0xaf,
                              3,
                              'f',
                              'o',
                              'o',
                              6,
                              'b',
                              'a',
                              'r',
                              'b',
                              'a',
                              'r',
                          }));
}

TEST_F(StreamFixture, ExtractIntoAll) {
  Stream st;
  std::vector<u8> a = {'a', 'b'};

  st << (int)0xdeadbeef;
  st << (short)0xafac;
  st << std::string("foo");
  st << std::string("barbar");

  EXPECT_TRUE(st);

  st.extract_into(a);
  EXPECT_EQ(a, std::vector<u8>({
                   'a', 'b', 0xef, 0xbe, 0xad, 0xde, 0xac, 0xaf, 3,   'f',
                   'o', 'o', 6,    'b',  'a',  'r',  'b',  'a',  'r',
               }));
}

class StreamFixturePrinter : public StreamFixture {
protected:
  std::string_view view(const Stream &st) {
    // reset/clean output_
    std::stringstream().swap(output_);

    output_ << st;

    return output_.view();
  }

  std::stringstream output_;
};

TEST_F(StreamFixturePrinter, Write) {
  Stream st;

  st << (uint64_t)42;
  st << (uint32_t)42;
  st << (uint16_t)42;
  st << (uint8_t)42;
  st << (int64_t)-42;
  st << (int32_t)-42;
  st << (int16_t)-42;
  st << (int8_t)-42;
  st << std::string("abcABC");
  st << std::string("foo");
  st << std::string("bar");

  EXPECT_TRUE(st);
  EXPECT_EQ(view(st), "2a00000000000000"
                      "2a000000"
                      "2a00"
                      "2a"
                      "d6ffffffffffffff"
                      "d6ffffff"
                      "d6ff"
                      "d6"
                      "06616263414243"
                      "03666f6f"
                      "03626172");
}

TEST_F(StreamFixturePrinter, Read) {
  Stream st;

  st << std::vector<u8>({6, 'a', 'b', 'c', 'A', 'B', 'C'})
     << std::vector<u8>({3, 'f', 'o', 'o'})
     << std::vector<u8>({3, 'b', 'a', 'r'})
     << std::vector<u8>({0xef, 0xbe, 0xad, 0xde});

  EXPECT_TRUE(st);

  std::string abc, foo, bar;
  unsigned int deadbeef;

  st >> abc >> foo >> bar >> deadbeef;

  EXPECT_TRUE(st);

  EXPECT_EQ(abc, std::string("abcABC"));
  EXPECT_EQ(foo, std::string("foo"));
  EXPECT_EQ(bar, std::string("bar"));
  EXPECT_EQ(deadbeef, 0xdeadbeef);
}

TEST_F(StreamFixturePrinter, Ranges) {
  Stream payload;
  auto p1 = std::vector<u8>({0, 0, 0, 0, 0x1, 0x2a, 0x0, 0x0, 0x0});
  auto p2 = std::vector<u8>({0, 0, 0, 0, 5, 'h', 'e', 'l', 'l', 'o'});

  // remove headers
  payload << (p1 | std::views::drop(4)) << (p2 | std::views::drop(4));

  EXPECT_TRUE(payload);
  EXPECT_EQ(view(payload), "012a0000000568656c6c6f");

  u8 status;
  int id;
  std::string s;

  payload >> status >> id >> s;

  EXPECT_TRUE(payload);

  EXPECT_EQ(status, 1);
  EXPECT_EQ(id, 42);
  EXPECT_EQ(s, std::string("hello"));
}

TEST_F(StreamFixturePrinter, Swap) {
  static Stream payload;
  auto p1 = std::vector<u8>({0, 0, 0, 0, 0x1, 0x2a, 0x0, 0x0, 0x0});
  auto p2 = std::vector<u8>({0, 0, 0, 0, 5, 'h', 'e', 'l', 'l', 'o'});

  // remove headers
  payload << (p1 | std::views::drop(4)) << (p2 | std::views::drop(4));

  EXPECT_TRUE(payload);

  u8 status;
  int id;
  std::string s;

  payload >> status >> id;
  EXPECT_TRUE(payload);
  EXPECT_EQ(status, 1);
  EXPECT_EQ(id, 42);

  Stream copy;
  copy.swap(payload);

  EXPECT_EQ(view(copy), "012a0000000568656c6c6f");
  EXPECT_EQ(view(payload), "");

  copy >> s;
  EXPECT_TRUE(copy);
  EXPECT_EQ(s, std::string("hello"));
}

TEST(Split, Split) {
  Stream payload;

  payload << (uint32_t)42;
  payload << (uint16_t)42;
  payload << (uint8_t)42;
  payload << std::string("foo");
  payload << std::string("bar");

  EXPECT_TRUE(payload);

  std::vector<std::vector<u8>> ps;

  // split the payload
  while (payload) {
    auto header = std::vector<u8>(3);
    payload.extract_into(header, 7);
    ps.push_back(std::move(header));
  }

  ASSERT_EQ(ps.size(), 3u);
  ASSERT_EQ(ps[0],
            std::vector<u8>({0, 0, 0, 0x2a, 0x0, 0x0, 0x0, 0x2a, 0x0, 0x2a}));
  ASSERT_EQ(ps[1], std::vector<u8>({0, 0, 0, 3, 'f', 'o', 'o', 3, 'b', 'a'}));
  ASSERT_EQ(ps[2], std::vector<u8>({0, 0, 0, 'r'}));
}

struct Foo {
  int id;
  std::string name;

  bool operator==(const Foo &other) const {
    return id == other.id && name == other.name;
  }

  friend Stream &operator<<(Stream &st, const Foo &foo) {
    return st << foo.id << foo.name;
  }

  friend Stream &operator>>(Stream &st, Foo &foo) {
    return st >> foo.id >> foo.name;
  }
};

TEST_F(StreamFixturePrinter, SerializeDeserialize) {
  Stream st;
  Foo a{.id = 42, .name = "I am a struct Foo"}, b{.id = 13, .name = "me too"};
  Foo A, B;

  st << a << b;
  EXPECT_TRUE(st);

  st >> A >> B;
  EXPECT_TRUE(st);

  ASSERT_EQ(a, A);
  ASSERT_EQ(b, B);

  ASSERT_EQ(view(st), std::string("2a000000"
                                  "114920616d20612073747275637420466f6f"
                                  "0d000000"
                                  "066d6520746f6f"));
}
