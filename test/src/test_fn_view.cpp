#include "static_assert.hpp"
#include <veg/fn_view.hpp>
#include <veg/option.hpp>
#include <gtest/gtest.h>

#define NOEXCEPT __VEG_CPP17(noexcept)

using namespace veg;
TEST(function_view, no_args) {
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
			meta::internal::equality_comparable_impl<T, std::nullptr_t>::value);

	void (*inc_fn_ptr)() noexcept = +[]() noexcept { ++global; };
	auto inc2_global_lambda = []() noexcept { global += 2; };
	auto returns_fn_ptr = +[]() noexcept {
		global += 3;
		return global;
	};

	fn_view<void() NOEXCEPT> f{inc_lambda};
	EXPECT_EQ(i, 0);
	f();
	EXPECT_EQ(i, 1);
	f();
	EXPECT_EQ(i, 2);
	fn_view<void()>{f}();
	EXPECT_EQ(i, 3);
	once_fn_view<void()>{f}();
	EXPECT_EQ(i, 4);

	STATIC_ASSERT(
			!std::is_constructible<fn_view<void()>, once_fn_view<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<once_fn_view<void()>, once_fn_view<void()>>::value);
	STATIC_ASSERT(
			std::is_constructible<once_fn_view<void()>, fn_view<void() NOEXCEPT>>::
					value);
	__VEG_CPP17(STATIC_ASSERT(!std::is_constructible<
														once_fn_view<void() noexcept>,
														fn_view<void()>>::value);)

	f = inc2_lambda;
	f();
	EXPECT_EQ(i, 6);

	f = returns_lambda;
	f();
	EXPECT_EQ(i, 9);

	f = *inc_fn_ptr;
	EXPECT_EQ(global, 0);
	f();
	EXPECT_EQ(global, 1);

	f = inc2_global_lambda;
	f();
	EXPECT_EQ(global, 3);

	f = +returns_fn_ptr;
	f = *+returns_fn_ptr;
	f();
	EXPECT_EQ(global, 6);
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

TEST(function_view, member_functions) {
	foo a;
	foo b;
	fn_view<foo(foo&, foo, int)> fn(&foo::bar);

	foo _ = fn(a, {}, 1);
	EXPECT_EQ(global, &a);
	fn = &foo::bar2;
	_ = fn(b, {}, 1);
	EXPECT_EQ(global, (&b + 1));

	fn = baz;
	_ = fn(b, {}, 1);
	EXPECT_EQ(global, nullptr);
}

TEST(function_view, null) {
	option<fn_view<void()>> f;
	STATIC_ASSERT(sizeof(f) == sizeof(VEG_FWD(f).unwrap()));

	EXPECT_TRUE(!f);
	EXPECT_DEATH({ void(f.as_ref().unwrap()); }, "");
	EXPECT_DEATH({ void(VEG_MOV(f).unwrap()); }, "");

	f = {some, [] {}};
	EXPECT_TRUE(f);

	void (*null)() = nullptr;

	auto make_null = [&] { f.as_ref().unwrap() = {null}; };
	EXPECT_DEATH({ make_null(); }, "");
}
