#define __VEG_DISABLE_NOEXCEPT
#include <veg/util/index.hpp>
#include <ostream>
#include <catch.hpp>
#include <veg/internal/prologue.hpp>

TEST_CASE("index: all") {
	using namespace veg;
	using namespace veg::literals;

	static_assert(std::is_constructible<Fix<3>, Fix<3>>::value, "fail");
	static_assert(std::is_constructible<Fix<3>, Dyn>::value, "fail");
	static_assert(std::is_constructible<Dyn, Dyn>::value, "fail");
	static_assert(std::is_constructible<Dyn, Fix<3>>::value, "fail");
	static_assert(std::is_constructible<Dyn, i64>::value, "fail");
	static_assert(std::is_constructible<Fix<3>, i64>::value, "fail");

	static_assert(!std::is_constructible<Fix<3>, Fix<4>>::value, "fail");

	CHECK(0_c != 2_c);
	CHECK(0_c + 2_c == 2_c);
	CHECK(0_c + Dyn(2_c) == 2_c);
	CHECK(Dyn(0_c) + Dyn(2_c) == Dyn(2_c));
	CHECK(0_v + 2_v == 2_v);
	CHECK(2_c - 2_c == 0_c);
	CHECK(2_c - 1_c - 1_c - 1_c == -Dyn(1_c));

	CHECK(Boolean<maybe>(true));
	CHECK(!Boolean<maybe>(false));
	CHECK(!Boolean<maybe>());
	CHECK(!Boolean<no>());
	CHECK(Boolean<yes>());
	CHECK_THROWS(Fix<2>{3});
	CHECK_THROWS(void(2_c / 0_v));

	VEG_ASSERT(0_v < 2_c);
	CHECK_THROWS(VEG_INTERNAL_ASSERT_PRECONDITION(0_v > 2_c));
	CHECK_THROWS(VEG_INTERNAL_ASSERT_PRECONDITION(0_v == 2_c));
}
#include <veg/internal/epilogue.hpp>
