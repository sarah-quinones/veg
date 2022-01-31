#include "static_assert.hpp"
#include <veg/fn_dyn.hpp>
#include <veg/option.hpp>
#include <doctest.h>
#include "veg/internal/prologue.hpp"

using namespace veg::fn;
using namespace veg;
TEST_CASE("function_view: no_args") {
	static int global = 0;
	using FnRef = FnRefDyn<MayThrow<void() const&>>;
	int i = 0;
	auto inc_lambda = tuplify([&i]() noexcept { ++i; });
	auto inc2_lambda = tuplify([&i]() noexcept { i += 2; });
	auto returns_lambda = tuplify([&i]() noexcept { i += 3; });

	auto inc2_global_lambda = tuplify([]() noexcept { global += 2; });

	FnRef f{from_i, ref(inc_lambda)};
	CHECK(i == 0);
	f();
	CHECK(i == 1);
	f();
	CHECK(i == 2);
	FnRef{f}();
	CHECK(i == 3);

	f = {from_i, ref(inc2_lambda)};
	f();
	CHECK(i == 5);

	f = {from_i, ref(returns_lambda)};
	f();
	CHECK(i == 8);

	f = FnRef{from_i, ref(inc2_global_lambda)};
	f();
	CHECK(global == 2);
}

static void const*
		global = // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		nullptr;

struct foo {
	auto bar(foo /*unused*/, int /*unused*/) & noexcept -> foo {
		global = this;
		return {};
	}
	auto bar2(foo /*unused*/, int /*unused*/) noexcept -> foo {
		global = this + 1;
		return {};
	}
};

auto baz(foo const& /*unused*/, foo /*unused*/, int /*unused*/) -> foo {
	global = nullptr;
	return {};
}
#include "veg/internal/epilogue.hpp"
