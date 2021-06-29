#include <veg/functional/utils.hpp>
#include <veg/uwunion.hpp>
#include "static_assert.hpp"
#include <doctest.h>
#include "veg/internal/prologue.hpp"

using namespace veg;

TEST_CASE("trivial") {

	constexpr auto u1 = Uwunion<int, float>{0_c, 2624};
	constexpr auto u2 = Uwunion<int, float>{1_c, 0.2F};

	constexpr struct {
		constexpr auto operator()(int i) const -> double { return double(i) / 2.0; }
		constexpr auto operator()(float f) const -> float { return 4.0F * f; }
	} fn;

	STATIC_ASSERT_IF_14(u1.as_ref().map(fn).unwrap(0_c) == 1312.0);
	STATIC_ASSERT_IF_14(u2.as_ref().map(fn).unwrap(1_c) == 0.8F);

	constexpr struct {
		constexpr auto operator()(int i) const -> int { return i / 2; }
		constexpr auto operator()(float f) const -> int { return int(16.0F * f); }
	} visitor;

	STATIC_ASSERT_IF_14(u1.as_ref().visit(visitor) == 1312);
	STATIC_ASSERT_IF_14(u2.as_ref().visit(visitor) == 3);
}

TEST_CASE("non trivial") {

	struct S { // NOLINT
		VEG_CPP20(constexpr) S() { (void)this; }
		VEG_CPP20(constexpr) S(S&& /*rhs*/) {}
		VEG_CPP20(constexpr) ~S() { (void)this; }
		int inner = 111;
	};

	VEG_CPP20(constexpr) auto u1 = Uwunion<int, int, S>{0_c, 2624};
	VEG_CPP20(constexpr) auto u2 = Uwunion<int, int, S>{1_c, 13};
	VEG_CPP20(constexpr) auto u3 = Uwunion<int, int, S>{2_c, {}};

	VEG_CPP20(constexpr)
	auto fn_idx = overload(
			[](Fix<0> /*tag*/, int i) -> double { return double(i) / 2.0; },
			[](Fix<1> /*tag*/, int i) -> int { return 4 * i; },
			[](Fix<2> /*tag*/, S const& s) -> int { return s.inner; });

	STATIC_ASSERT_IF_20(u1.as_ref().map_i(fn_idx).unwrap(0_c) == 1312.0);
	STATIC_ASSERT_IF_20(u2.as_ref().map_i(fn_idx).unwrap(1_c) == 52);
	STATIC_ASSERT_IF_20(u3.as_ref().map_i(fn_idx).unwrap(2_c) == 111);

	VEG_CPP20(constexpr)
	auto visitor_idx = overload(
			[](Fix<0> /*tag*/, int i) -> int { return i / 2; },
			[](Fix<1> /*tag*/, int f) -> int { return 16 * f; },
			[](Fix<2> /*tag*/, S const& s) -> int { return s.inner; });

	STATIC_ASSERT_IF_20(u1.as_ref().visit_i(visitor_idx) == 1312);
	STATIC_ASSERT_IF_20(u2.as_ref().visit_i(visitor_idx) == 208);
	STATIC_ASSERT_IF_20(u3.as_ref().visit_i(visitor_idx) == 111);
}

#include "veg/internal/epilogue.hpp"
