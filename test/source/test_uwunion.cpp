#include <veg/functional/overload.hpp>
#include <veg/uwunion.hpp>
#include <veg/tuple.hpp>
#include <veg/functional/compose.hpp>
#include <veg/functional/ref.hpp>

#include "static_assert.hpp"
#include <catch.hpp>

#include "veg/internal/prologue.hpp"

using namespace veg;

TEST_CASE("trivial") {

	constexpr auto u1 = Uwunion<int, float>{0_c, 2624};
	constexpr auto u2 = Uwunion<int, float>{1_c, 0.2F};

	constexpr struct {
		constexpr auto operator()(Ref<int> i) const -> double {
			return double(*i) / 2.0;
		}
		constexpr auto operator()(Ref<float> f) const -> float { return 4.0F * *f; }
	} fn;

	STATIC_ASSERT(u1 == u1);
	STATIC_ASSERT(cmp::cmp(ref(u1), ref(u1)) == cmp::Ordering::equal);

	STATIC_ASSERT_IF_14(u1.as_ref().map(fn).unwrap(0_c) == 1312.0);
	STATIC_ASSERT_IF_14(u2.as_ref().map(fn).unwrap(1_c) == 0.8F);

	constexpr struct {
		constexpr auto operator()(Ref<int> i) const -> int { return *i / 2; }
		constexpr auto operator()(Ref<float> f) const -> int {
			return int(16.0F * *f);
		}
	} visitor;

	STATIC_ASSERT_IF_14(u1.as_ref().visit(visitor) == 1312);
	STATIC_ASSERT_IF_14(u2.as_ref().visit(visitor) == 3);
}

#if defined(__clang__)
#define CONSTEXPR VEG_CPP20(constexpr)
#else
#define CONSTEXPR
#endif

TEST_CASE("non trivial") {

	struct S { // NOLINT
		CONSTEXPR S() { (void)this; }
		CONSTEXPR S(S&& /*rhs*/) {}
		CONSTEXPR ~S() { (void)this; }
		int inner = 111;
	};

	static CONSTEXPR auto u1 = Uwunion<int, int, S>{0_c, 2624};
	CONSTEXPR auto u2 = Uwunion<int, int, S>{1_c, 13};
	CONSTEXPR auto u3 = Uwunion<int, int, S>{2_c, {}};

	auto copy = fn::compose(veg::clone, veg::ref);

	CONSTEXPR
	auto fn_idx = tuplify(
			[](Ref<int> i) -> double { return double(*i) / 2.0; },
			[](Ref<int> i) -> int { return 4 * *i; },
			[](Ref<S> s) -> int { return (*s).inner; });

	STATIC_ASSERT_IF_20(u1.as_ref().map_i(copy(fn_idx)).unwrap(0_c) == 1312.0);
	STATIC_ASSERT_IF_20(u2.as_ref().map_i(copy(fn_idx)).unwrap(1_c) == 52);
	STATIC_ASSERT_IF_20(u3.as_ref().map_i(copy(fn_idx)).unwrap(2_c) == 111);

	CONSTEXPR
	auto visitor_idx = tuplify(
			[](Ref<int> i) -> int { return *i / 2; },
			[](Ref<int> f) -> int { return 16 * *f; },
			[](Ref<S> s) -> int { return (*s).inner; });

	STATIC_ASSERT_IF_20(u1.as_ref().visit_i(copy(visitor_idx)) == 1312);
	STATIC_ASSERT_IF_20(u2.as_ref().visit_i(copy(visitor_idx)) == 208);
	STATIC_ASSERT_IF_20(u3.as_ref().visit_i(copy(visitor_idx)) == 111);

	CONSTEXPR auto test =
			const_cast<veg::meta::uncvref_t<decltype(u1)>&>(u1).as_mut();
	veg::unused(test);
}

#include "veg/internal/epilogue.hpp"
