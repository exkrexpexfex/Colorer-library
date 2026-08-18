#include <catch2/catch_amalgamated.hpp>
#include "colorer/HrcLibrary.h"
#include "colorer/LineSource.h"
#include "colorer/TextParser.h"
#include "colorer/handlers/LineRegionsCompactSupport.h"
#include "colorer/handlers/LineRegionsSupport.h"
#include "colorer/handlers/StyledHRDMapper.h"
#include "colorer/handlers/StyledRegion.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

class SingleLineSource : public LineSource
{
 public:
  explicit SingleLineSource(UnicodeString line) : line_(std::move(line)) {}

  UnicodeString* getLine(size_t lno) override
  {
    return lno == 0 ? &line_ : nullptr;
  }

 private:
  UnicodeString line_;
};

void parseInto(LineRegionsSupport& support, HrcLibrary& lib, const UnicodeString& type_name, const UnicodeString& line)
{
  auto* file_type = lib.getFileType(type_name);
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  SingleLineSource source(line);
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&support);
  parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_OFF);
}

const LineRegion* findRegion(const LineRegion* head, const UnicodeString& name)
{
  for (const LineRegion* lr = head; lr != nullptr; lr = lr->next) {
    if (lr->region != nullptr && lr->region->getName().compare(name) == 0) {
      return lr;
    }
  }
  return nullptr;
}

}  // namespace

TEST_CASE("LineRegion reuses rdef storage and still assigns parent fill", "[lineregions]")
{
  LineRegion lr;
  const StyledRegion src(true, false, 0xFF0000, 0, StyledRegion::RD_BOLD);
  const StyledRegion parent(true, true, 0x1, 0x222222, StyledRegion::RD_NONE);

  lr.applyRegionDefine(&src, &parent);
  REQUIRE(lr.styled()->fore == 0xFF0000);
  REQUIRE(lr.styled()->isForeSet);
  REQUIRE(lr.styled()->back == 0x222222);
  REQUIRE(lr.styled()->isBackSet);
  REQUIRE(lr.styled()->style == StyledRegion::RD_BOLD);

  const RegionDefine* first = lr.rdef;
  const StyledRegion src2(true, false, 0x00FF00, 0, StyledRegion::RD_ITALIC);
  lr.applyRegionDefine(&src2, &parent);
  REQUIRE(lr.rdef == first);
  REQUIRE(lr.styled()->fore == 0x00FF00);
  REQUIRE(lr.styled()->back == 0x222222);
  REQUIRE(lr.styled()->style == StyledRegion::RD_ITALIC);
}

TEST_CASE("LineRegionsSupport keeps HRD parent fill after recycling regions", "[lineregions]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_kwcase.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  const StyledRegion background(true, true, 0xAAAAAA, 0x111111, StyledRegion::RD_NONE);
  const StyledRegion keyword(true, false, 0xFF0000, 0, StyledRegion::RD_BOLD);

  StyledHRDMapper mapper;
  mapper.setRegionDefine(UnicodeString("kw_sensitive:Keyword"), &keyword);

  auto check_line = [&](LineRegionsSupport& support) {
    support.resize(1);
    support.setRegionMapper(&mapper);
    support.setBackground(&background);
    parseInto(support, lib, UnicodeString("kw_sensitive"), UnicodeString(u"If if"));

    const LineRegion* kw = findRegion(support.getLineRegions(0), UnicodeString("kw_sensitive:Keyword"));
    REQUIRE(kw != nullptr);
    REQUIRE(kw->start == 0);
    REQUIRE(kw->end == 2);
    REQUIRE(kw->styled()->fore == 0xFF0000);
    REQUIRE(kw->styled()->back == 0x111111);
    REQUIRE(kw->styled()->style == StyledRegion::RD_BOLD);
  };

  SECTION("plain store")
  {
    LineRegionsSupport support;
    check_line(support);
    parseInto(support, lib, UnicodeString("kw_sensitive"), UnicodeString(u"If if"));
    const LineRegion* kw = findRegion(support.getLineRegions(0), UnicodeString("kw_sensitive:Keyword"));
    REQUIRE(kw != nullptr);
    REQUIRE(kw->styled()->fore == 0xFF0000);
    REQUIRE(kw->styled()->back == 0x111111);
  }

  SECTION("compact store")
  {
    LineRegionsCompactSupport support;
    check_line(support);
    parseInto(support, lib, UnicodeString("kw_sensitive"), UnicodeString(u"If if"));
    const LineRegion* kw = findRegion(support.getLineRegions(0), UnicodeString("kw_sensitive:Keyword"));
    REQUIRE(kw != nullptr);
    REQUIRE(kw->styled()->back == 0x111111);
  }
}
