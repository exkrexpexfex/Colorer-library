#include <colorer/Common.h>
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("UnicodeString compare", "[ustr]")
{
  const UnicodeString abc(u"abc");
  const UnicodeString abd(u"abd");
  const UnicodeString ab(u"ab");
  const UnicodeString empty(u"");
  const UnicodeString prefix(u"xxabcyy");

  SECTION("equal strings")
  {
    REQUIRE(abc.compare(UnicodeString(u"abc")) == 0);
    REQUIRE(empty.compare(empty) == 0);
  }

  SECTION("first mismatch decides order")
  {
    REQUIRE(abc.compare(abd) < 0);
    REQUIRE(abd.compare(abc) > 0);
  }

  SECTION("common prefix, shorter string is less")
  {
    REQUIRE(ab.compare(abc) < 0);
    REQUIRE(abc.compare(ab) > 0);
    REQUIRE(empty.compare(abc) < 0);
    REQUIRE(abc.compare(empty) > 0);
  }

  SECTION("slice against a whole string")
  {
    REQUIRE(prefix.compare(2, 3, abc) == 0);
    REQUIRE(prefix.compare(2, 3, abd) < 0);
    REQUIRE(prefix.compare(2, 2, ab) == 0);
    REQUIRE(prefix.compare(2, 2, abc) < 0);
    REQUIRE(prefix.compare(2, 4, abc) > 0);
  }
}

TEST_CASE("UnicodeString caseCompare", "[ustr]")
{
  const UnicodeString abc(u"Abc");
  const UnicodeString abd(u"aBd");
  const UnicodeString ab(u"AB");
  const UnicodeString empty(u"");
  const UnicodeString prefix(u"xxAbCyy");

  SECTION("equal ignoring case")
  {
    REQUIRE(UStr::caseCompare(abc, UnicodeString(u"abc")) == 0);
    REQUIRE(UStr::caseCompare(empty, empty) == 0);
  }

  SECTION("first mismatch decides order")
  {
    REQUIRE(UStr::caseCompare(abc, abd) < 0);
    REQUIRE(UStr::caseCompare(abd, abc) > 0);
  }

  SECTION("common prefix, shorter string is less")
  {
    REQUIRE(UStr::caseCompare(ab, abc) < 0);
    REQUIRE(UStr::caseCompare(abc, ab) > 0);
    REQUIRE(UStr::caseCompare(empty, abc) < 0);
    REQUIRE(UStr::caseCompare(abc, empty) > 0);
  }

  SECTION("slice against a whole string")
  {
    REQUIRE(UStr::caseCompare(prefix, 2, 3, UnicodeString(u"abc")) == 0);
    REQUIRE(UStr::caseCompare(prefix, 2, 3, UnicodeString(u"abd")) < 0);
    REQUIRE(UStr::caseCompare(prefix, 2, 2, UnicodeString(u"ab")) == 0);
    REQUIRE(UStr::caseCompare(prefix, 2, 2, UnicodeString(u"abc")) < 0);
    REQUIRE(UStr::caseCompare(prefix, 2, 4, UnicodeString(u"abc")) > 0);
  }
}

TEST_CASE("UStr createCharClass ignore_case", "[ustr]")
{
  SECTION("single letter folds both cases")
  {
    int end = -1;
    const auto cls = UStr::createCharClass(UnicodeString(u"[x]"), 0, &end, true);
    REQUIRE(cls != nullptr);
    REQUIRE(cls->contains(u'x'));
    REQUIRE(cls->contains(u'X'));
    REQUIRE(end == 2);
  }

  SECTION("later letters fold themselves, not the previous character")
  {
    int end = -1;
    const auto cls = UStr::createCharClass(UnicodeString(u"[aX]"), 0, &end, true);
    REQUIRE(cls != nullptr);
    REQUIRE(cls->contains(u'a'));
    REQUIRE(cls->contains(u'A'));
    REQUIRE(cls->contains(u'X'));
    REQUIRE(cls->contains(u'x'));
  }

  SECTION("without ignore_case stays case sensitive")
  {
    int end = -1;
    const auto cls = UStr::createCharClass(UnicodeString(u"[x]"), 0, &end, false);
    REQUIRE(cls != nullptr);
    REQUIRE(cls->contains(u'x'));
    REQUIRE_FALSE(cls->contains(u'X'));
  }

  SECTION("escaped letter also folds")
  {
    int end = -1;
    const auto cls = UStr::createCharClass(UnicodeString(u"[\\z]"), 0, &end, true);
    REQUIRE(cls != nullptr);
    REQUIRE(cls->contains(u'z'));
    REQUIRE(cls->contains(u'Z'));
  }
}
