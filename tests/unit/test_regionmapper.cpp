#include <colorer/Region.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/handlers/StyledRegion.h>
#include <catch2/catch_amalgamated.hpp>

namespace {

UnicodeString ustr(const char16_t* text)
{
  return UnicodeString(text);
}

}  // namespace

TEST_CASE("RegionMapper cache lookup", "[regionmapper]")
{
  StyledHRDMapper mapper;
  const StyledRegion def(true, true, 0x112233, 0x445566, StyledRegion::RD_NONE);
  mapper.setRegionDefine(ustr(u"def:Text"), &def);

  SECTION("region id 0 with a mapping does not throw and returns the define")
  {
    const Region text(ustr(u"def:Text"), nullptr, nullptr, 0);
    const RegionDefine* rd = nullptr;
    REQUIRE_NOTHROW(rd = mapper.getRegionDefine(&text));
    REQUIRE(rd != nullptr);
    const auto* styled = StyledRegion::cast(rd);
    REQUIRE(styled->fore == 0x112233);
    REQUIRE(styled->back == 0x445566);
  }

  SECTION("second lookup of id 0 hits the cache")
  {
    const Region text(ustr(u"def:Text"), nullptr, nullptr, 0);
    const auto* first = mapper.getRegionDefine(&text);
    const auto* second = mapper.getRegionDefine(&text);
    REQUIRE(first == second);
  }

  SECTION("unmapped region id 0 without parent returns nullptr")
  {
    const Region unknown(ustr(u"def:Unknown"), nullptr, nullptr, 0);
    REQUIRE(mapper.getRegionDefine(&unknown) == nullptr);
  }

  SECTION("unmapped child inherits parent define through id 0")
  {
    const Region parent(ustr(u"def:Text"), nullptr, nullptr, 0);
    const Region child(ustr(u"cpp:Keyword"), nullptr, &parent, 1);
    const auto* rd = mapper.getRegionDefine(&child);
    REQUIRE(rd != nullptr);
    REQUIRE(StyledRegion::cast(rd)->fore == 0x112233);
    REQUIRE(mapper.getRegionDefine(&parent) == rd);
  }

  SECTION("high region id grows the empty cache")
  {
    const Region text(ustr(u"def:Text"), nullptr, nullptr, 5);
    const auto* rd = mapper.getRegionDefine(&text);
    REQUIRE(rd != nullptr);
    REQUIRE(StyledRegion::cast(rd)->fore == 0x112233);
  }

  SECTION("null region returns nullptr")
  {
    REQUIRE(mapper.getRegionDefine(static_cast<const Region*>(nullptr)) == nullptr);
  }
}
