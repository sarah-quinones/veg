#include "assert_death.hpp"
#include "static_assert.hpp"
#include <veg/fn_view.hpp>
#include <veg/option.hpp>
#include <doctest.h>
#include "veg/internal/prologue.hpp"

using namespace veg::fn;
TEST_CASE("function_view, no_args") {
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
	STATIC_ASSERT(
			veg::meta::internal::equality_comparable_impl<T, std::nullptr_t>::value);

	void (*inc_fn_ptr)() noexcept = +[]() noexcept { ++global; };
	auto inc2_global_lambda = []() noexcept { global += 2; };
	auto returns_fn_ptr = +[]() noexcept {
		global += 3;
		return global;
	};

#if __cplusplus >= 201703L
	fn_view<nothrow<void()>> f{inc_lambda};
#else
	fn_view<void()> f{inc_lambda};
#endif
	CHECK(i == 0);
	f();
	CHECK(i == 1);
	f();
	CHECK(i == 2);
	fn_view<void()>{f}();
	CHECK(i == 3);
	once_fn_view<void()>{f}();
	CHECK(i == 4);

	STATIC_ASSERT(
			!std::is_constructible<fn_view<void()>, once_fn_view<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<once_fn_view<void()>, once_fn_view<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<once_fn_view<void()>, fn_view<nothrow<void()>>>::
					value);
	VEG_CPP17(STATIC_ASSERT(!std::is_constructible<
													once_fn_view<nothrow<void()>>,
													fn_view<void()>>::value);)

	f = inc2_lambda;
	f();
	CHECK(i == 6);

	f = returns_lambda;
	f();
	CHECK(i == 9);

	f = *inc_fn_ptr;
	CHECK(global == 0);
	f();
	CHECK(global == 1);

	f = inc2_global_lambda;
	f();
	CHECK(global == 3);

	f = +returns_fn_ptr;
	f = *+returns_fn_ptr;
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

TEST_CASE("function_view, null") {
	veg::option<fn_view<void()>> f;
	STATIC_ASSERT(sizeof(f) == sizeof(VEG_FWD(f).unwrap()));

	CHECK(!f);
	CHECK_DEATH({ void(f.as_ref().unwrap()); });
	CHECK_DEATH({ void(VEG_MOV(f).unwrap()); });

	f = {veg::some, [] {}};
	CHECK(f);

	void (*null)() = nullptr;

	auto make_null = [&] { f.as_ref().unwrap() = {null}; };
	CHECK_DEATH({ make_null(); });
}
#include "veg/internal/epilogue.hpp"
