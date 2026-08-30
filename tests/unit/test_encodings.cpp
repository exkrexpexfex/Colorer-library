#include <catch2/catch_amalgamated.hpp>
#include <initializer_list>
#include "colorer/Common.h"

namespace {

void require_bytes(const byte* buf, int n, std::initializer_list<byte> expected)
{
  REQUIRE(n == static_cast<int>(expected.size()));
  int i = 0;
  for (byte b : expected) {
    REQUIRE(buf[i] == b);
    i++;
  }
}

}  // namespace

TEST_CASE("Encodings toUTF8Bytes BMP", "[encodings]")
{
  byte buf[8] = {};

  SECTION("ASCII")
  {
    require_bytes(buf, Encodings::toUTF8Bytes(U'A', buf), {0x41});
  }

  SECTION("two-byte")
  {
    require_bytes(buf, Encodings::toUTF8Bytes(0x00A9, buf), {0xC2, 0xA9});
  }

  SECTION("three-byte")
  {
    require_bytes(buf, Encodings::toUTF8Bytes(0x20AC, buf), {0xE2, 0x82, 0xAC});
  }
}

#ifndef COLORER_FEATURE_ICU

TEST_CASE("Encodings toUTF8Bytes supplementary plane", "[encodings]")
{
  byte buf[8] = {};
  require_bytes(buf, Encodings::toUTF8Bytes(static_cast<wchar>(0x10000), buf), {0xF0, 0x90, 0x80, 0x80});
  require_bytes(buf, Encodings::toUTF8Bytes(static_cast<wchar>(0x1F600), buf), {0xF0, 0x9F, 0x98, 0x80});
  require_bytes(buf, Encodings::toUTF8Bytes(static_cast<wchar>(0x10FFFF), buf), {0xF4, 0x8F, 0xBF, 0xBF});
}

#if (__WCHAR_MAX__ > 0xffff)

TEST_CASE("Encodings toBytes UTF-32", "[encodings]")
{
  byte buf[8] = {};
  const wchar grinning = static_cast<wchar>(0x1F600);

  SECTION("little endian")
  {
    require_bytes(buf, Encodings::toBytes(Encodings::ENC_UTF32, grinning, buf), {0x00, 0xF6, 0x01, 0x00});
  }

  SECTION("big endian")
  {
    require_bytes(buf, Encodings::toBytes(Encodings::ENC_UTF32BE, grinning, buf), {0x00, 0x01, 0xF6, 0x00});
  }
}

#endif

TEST_CASE("UnicodeString getChars UTF-8 for supplementary plane", "[encodings]")
{
  UnicodeString text;
  text.append(static_cast<wchar>(0x1F600));
  const char* utf8 = text.getChars(Encodings::ENC_UTF8);
  REQUIRE(utf8 != nullptr);
  REQUIRE(static_cast<unsigned char>(utf8[0]) == 0xF0);
  REQUIRE(static_cast<unsigned char>(utf8[1]) == 0x9F);
  REQUIRE(static_cast<unsigned char>(utf8[2]) == 0x98);
  REQUIRE(static_cast<unsigned char>(utf8[3]) == 0x80);
  REQUIRE(utf8[4] == 0);
}

#endif
