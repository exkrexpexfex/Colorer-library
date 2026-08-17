#include <catch2/catch_amalgamated.hpp>
#include <vector>
#include "colorer/HrcLibrary.h"
#include "colorer/LineSource.h"
#include "colorer/Region.h"
#include "colorer/RegionHandler.h"
#include "colorer/TextParser.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

struct RegionHit
{
  int start = 0;
  int end = 0;
  UnicodeString name;
};

class CollectHandler : public RegionHandler
{
 public:
  std::vector<RegionHit> hits;

  void addRegion(size_t /*lno*/, UnicodeString* /*line*/, int sx, int ex, const Region* region) override
  {
    hits.push_back({sx, ex, region->getName()});
  }

  void enterScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/, const Region* /*region*/,
                   const Scheme* /*scheme*/) override
  {
  }

  void leaveScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/, const Region* /*region*/,
                   const Scheme* /*scheme*/) override
  {
  }
};

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

std::vector<RegionHit> parseLine(HrcLibrary& lib, const UnicodeString& type_name, const UnicodeString& line)
{
  auto* file_type = lib.getFileType(type_name);
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  SingleLineSource source(line);
  CollectHandler handler;
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&handler);
  parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_OFF);
  return handler.hits;
}

}  // namespace

TEST_CASE("Keyword matching still works with lazy line lowercase", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_kwcase.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  SECTION("case-sensitive keywords match only the stored case")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_sensitive"), UnicodeString(u"If if"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_sensitive:Keyword")) == 0);
  }

  SECTION("ignorecase keywords match both cases")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_ignore"), UnicodeString(u"If if"));
    REQUIRE(hits.size() == 2);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[1].start == 3);
    REQUIRE(hits[1].end == 5);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_ignore:Keyword")) == 0);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_ignore:Keyword")) == 0);
  }

  SECTION("lowercase is built even if ignorecase keywords are not first")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_mixed"), UnicodeString(u"If Then if then"));
    REQUIRE(hits.size() == 3);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_mixed:Kw")) == 0);
    REQUIRE(hits[1].start == 3);
    REQUIRE(hits[1].end == 7);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_mixed:KwI")) == 0);
    REQUIRE(hits[2].start == 11);
    REQUIRE(hits[2].end == 15);
    REQUIRE(hits[2].name.compare(UnicodeString("kw_mixed:KwI")) == 0);
  }
}
