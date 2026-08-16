#include <catch2/catch_amalgamated.hpp>
#include "test_common.h"

std::unique_ptr<TestLogger> logger;

TEST_CASE("All test cases reside in other .cpp files")
{
  logger = std::make_unique<TestLogger>();
  Log::registerLogger(*logger);
}