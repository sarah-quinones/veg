#include <string>

#include "veg/veg.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Name is veg", "[library]")
{
  exported_class e;
  REQUIRE(std::string("veg") == e.name());
}
