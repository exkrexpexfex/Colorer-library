#include <catch2/catch_amalgamated.hpp>
#include "test_common.h"

std::unique_ptr<TestLogger> logger;

namespace {

struct TestRunListener : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const& /*testRunInfo*/) override
  {
    logger = std::make_unique<TestLogger>();
    Log::registerLogger(*logger);
  }
};

}  // namespace

CATCH_REGISTER_LISTENER(TestRunListener)
