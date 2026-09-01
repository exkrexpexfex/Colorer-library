#include <catch2/catch_amalgamated.hpp>
#include "colorer/FileType.h"
#include "colorer/HrcLibrary.h"
#include "colorer/Scheme.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

XmlInputSource hrcInput(const char* file_name)
{
  auto path = fs::path(__FILE__).parent_path() / "data" / file_name;
  return XmlInputSource(UnicodeString(path.c_str()), nullptr);
}

}  // namespace

TEST_CASE("Load hrc", "[hrc]")
{
  XmlInputSource file1 = hrcInput("type_cue.hrc");
  HrcLibrary lib;
  lib.loadSource(&file1);
}

TEST_CASE("Load HRC expands a SYSTEM entity into the type tree", "[hrc]")
{
  XmlInputSource file = hrcInput("type_entity_incl.hrc");
  HrcLibrary lib;
  lib.loadSource(&file);
  auto* type = lib.getFileType(UnicodeString("entinc"));
  REQUIRE(type != nullptr);
  REQUIRE(type->getBaseScheme() != nullptr);
  REQUIRE(*type->getBaseScheme()->getName() == UnicodeString("entinc:entinc"));
}
