#include <colorer/Common.h>
#include <catch2/catch_amalgamated.hpp>

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
