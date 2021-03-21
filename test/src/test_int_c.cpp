#include "assert_death.hpp"
#include <veg/internal/meta_int.hpp>
#include <ostream>
#include <doctest.h>

TEST_CASE("meta_int, all") {
	using namespace veg;
	using namespace veg::literals;

	static_assert(std::is_constructible<fix<3>, fix<3>>::value, "fail");
	static_assert(std::is_constructible<fix<3>, dyn>::value, "fail");
	static_assert(std::is_constructible<dyn, dyn>::value, "fail");
	static_assert(std::is_constructible<dyn, fix<3>>::value, "fail");
	static_assert(std::is_constructible<dyn, i64>::value, "fail");
	static_assert(std::is_constructible<fix<3>, i64>::value, "fail");

	static_assert(!std::is_constructible<fix<3>, fix<4>>::value, "fail");

	CHECK(0_c != 2_c);
	CHECK(0_c + 2_c == 2_c);
	CHECK(0_c + dyn(2_c) == 2_c);
	CHECK(dyn(0_c) + dyn(2_c) == dyn(2_c));
	CHECK(0_v + 2_v == 2_v);
	CHECK(2_c - 2_c == 0_c);
	CHECK(2_c - 1_c - 1_c - 1_c == -dyn(1_c));

	CHECK(boolean<maybe>(true));
	CHECK(!boolean<maybe>(false));
	CHECK(!boolean<maybe>());
	CHECK(!boolean<no>());
	CHECK(boolean<yes>());
	CHECK_DEATH({ fix<2>{3}; });
	CHECK_DEATH({ void(2_c / 0_v); });
}
