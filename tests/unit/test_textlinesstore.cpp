#include <catch2/catch_amalgamated.hpp>
#include <fstream>
#include "colorer/utils/FileSystems.h"
#include "colorer/viewer/TextLinesStore.h"

TEST_CASE("TextLinesStore owns lines by value and splits CRLF", "[textlinesstore]")
{
  const auto path = fs::temp_directory_path() / "colorer_textlines.txt";
  {
    std::ofstream out(path, std::ios::binary);
    out << "a\r\nb\tc\n";
  }

  UnicodeString location(path.c_str());
  TextLinesStore store;
  store.loadFile(&location, true);

  REQUIRE(store.getFileName() != nullptr);
  REQUIRE(store.getLineCount() == 3);
  REQUIRE(*store.getLine(0) == UnicodeString("a"));
  REQUIRE(*store.getLine(1) == UnicodeString("b    c"));
  REQUIRE(*store.getLine(2) == UnicodeString(""));
  REQUIRE(store.getLine(3) == nullptr);

  const auto path2 = fs::temp_directory_path() / "colorer_textlines2.txt";
  {
    std::ofstream out(path2, std::ios::binary);
    out << "only";
  }
  UnicodeString location2(path2.c_str());
  store.loadFile(&location2, false);
  REQUIRE(store.getLineCount() == 1);
  REQUIRE(*store.getLine(0) == UnicodeString("only"));

  fs::remove(path);
  fs::remove(path2);
}
