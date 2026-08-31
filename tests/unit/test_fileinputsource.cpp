#include <catch2/catch_amalgamated.hpp>
#include <fstream>
#include <string>
#include "colorer/io/FileInputSource.h"
#include "colorer/utils/FileSystems.h"

TEST_CASE("FileInputSource reads a regular file", "[io]")
{
  const auto path = fs::temp_directory_path() / "colorer_q1_input.txt";
  {
    std::ofstream out(path, std::ios::binary);
    out << "hello";
  }

  const UnicodeString location(path.c_str());
  FileInputSource source(&location, nullptr);
  const byte* data = source.openStream();
  REQUIRE(source.length() == 5);
  REQUIRE(std::string(reinterpret_cast<const char*>(data), 5) == "hello");
  source.closeStream();
  fs::remove(path);
}

TEST_CASE("FileInputSource reads an empty file", "[io]")
{
  const auto path = fs::temp_directory_path() / "colorer_q1_empty.txt";
  {
    std::ofstream out(path, std::ios::binary);
  }

  const UnicodeString location(path.c_str());
  FileInputSource source(&location, nullptr);
  source.openStream();
  REQUIRE(source.length() == 0);
  source.closeStream();
  fs::remove(path);
}

TEST_CASE("FileInputSource missing file throws and can be retried", "[io]")
{
  const UnicodeString missing("/no/such/colorer_q1_missing_file");
  FileInputSource source(&missing, nullptr);
  REQUIRE_THROWS_WITH(source.openStream(), Catch::Matchers::ContainsSubstring("Can't open file"));
  REQUIRE_THROWS_AS(source.length(), InputSourceException);
}
