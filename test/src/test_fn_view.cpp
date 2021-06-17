#include <iostream>
#define __VEG_DISABLE_NOEXCEPT

#include "static_assert.hpp"
#include <veg/fn_view.hpp>
#include <veg/option.hpp>
#include <doctest.h>
#include "veg/internal/prologue.hpp"

using namespace veg::fn;
using namespace veg;
TEST_CASE("function_view: no_args") {
	static int global = 0;
	int i = 0;
	auto inc_lambda = [&i]() noexcept { ++i; };
	auto inc2_lambda = [&i]() noexcept { i += 2; };
	auto returns_lambda = [&i]() noexcept {
		i += 3;
		return i;
	};

	void (*p)(){};
	using T = decltype(p);

	STATIC_ASSERT(noexcept(p == nullptr));
	STATIC_ASSERT(VEG_CONCEPT(equality_comparable_with<T, std::nullptr_t>));

	void (*inc_fn_ptr)() noexcept = +[]() noexcept { ++global; };
	auto inc2_global_lambda = []() noexcept { global += 2; };
	auto returns_fn_ptr = +[]() noexcept {
		global += 3;
		return global;
	};

	FnView<void()> f{as_ref, inc_lambda};
	CHECK(i == 0);
	f();
	CHECK(i == 1);
	f();
	CHECK(i == 2);
	FnView<void()>{f}();
	CHECK(i == 3);
	FnOnceView<void()>{f}();
	CHECK(i == 4);

	STATIC_ASSERT(
			!std::is_constructible<FnView<void()>, FnOnceView<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<FnOnceView<void()>, FnOnceView<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<FnOnceView<void()>, FnView<void() noexcept>>::
					value);
	VEG_CPP17(STATIC_ASSERT(!std::is_constructible<
													FnOnceView<void() noexcept>,
													FnView<void()>>::value);)

	f = {as_ref, inc2_lambda};
	f();
	CHECK(i == 6);

	f = {as_ref, returns_lambda};
	f();
	CHECK(i == 9);

	f = {as_ref, *inc_fn_ptr};
	CHECK(global == 0);
	f();
	CHECK(global == 1);

	f = {as_ref, inc2_global_lambda};
	f();
	CHECK(global == 3);

	f = {as_ref, +returns_fn_ptr};
	f = {as_ref, *+returns_fn_ptr};
	f();
	CHECK(global == 6);
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

TEST_CASE("function_view: null") {
	{
		veg::Option<FnView<void()>> f;

		CHECK(f.is_none());
		CHECK_THROWS(void(f.as_ref().unwrap()));
		CHECK_THROWS(void(VEG_MOV(f).unwrap()));

		auto l = [] {};
		f = {some, {as_ref, l}};
		CHECK(f.is_some());
	}
	{
		void (*null)() = nullptr;
		CHECK_THROWS(FnView<void()>{as_ref, null});
	}
}
#include "veg/internal/epilogue.hpp"
