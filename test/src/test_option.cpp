#include "veg/option.hpp"
#include "veg/fn_view.hpp"
#include <gtest/gtest.h>
#include <vector>
#include "static_assert.hpp"

using namespace veg;
template <typename T, i64 N>
struct nested_option {
	using type = option<typename nested_option<T, N - 1>::type>;
};
template <typename T>
struct nested_option<T, 0> {
	using type = T;
};

TEST(option, all) {

	struct A {
		__VEG_CPP14(constexpr) auto operator()() const -> option<int> {
			return {some, 13};
		}
		__VEG_CPP14(constexpr) auto operator()(int i) const -> option<double> {
			if (i == 0) {
				return none;
			}
			return {some, 1000. / i};
		}
	};

	struct B {
		__VEG_CPP14(constexpr) auto operator()(int i) const -> double {
			if (i == 0) {
				return 1.0;
			}
			return 2000. / i;
		}
	};
	struct C {
		__VEG_CPP14(constexpr) auto operator()() const -> double { return 2000.; }
	};

	STATIC_ASSERT(sizeof(option<int>) == sizeof(int) * 2);
	STATIC_ASSERT(sizeof(option<option<int>>) == sizeof(int) * 2);
	STATIC_ASSERT(sizeof(option<option<option<option<int>>>>) == sizeof(int) * 2);
	STATIC_ASSERT(sizeof(option<fn_view<void()>>) == sizeof(fn_view<void()>));
	STATIC_ASSERT(sizeof(option<fn_view<void()>>) == sizeof(fn_view<void()>));

	__VEG_CPP14(constexpr) option<int> i = {some, 3};
	__VEG_CPP14(constexpr) option<int> j = none;
	STATIC_ASSERT_IF_14(i.as_ref().and_then(A{}).unwrap() == 1000. / 3);
	STATIC_ASSERT_IF_17(option<int>{some, inplace, [&] { return 0; }});
	STATIC_ASSERT_IF_14(i.as_ref().map(B{}).unwrap() == 2000. / 3);

	STATIC_ASSERT_IF_14(i.as_ref().map_or_else(B{}, C{}) == 2000. / 3);
	STATIC_ASSERT_IF_14(j.as_ref().map_or_else(B{}, C{}) == 2000.);
	STATIC_ASSERT_IF_14(j.as_ref().map_or(B{}, 2000.) == 2000.);
	STATIC_ASSERT_IF_14(i.as_ref().map_or(B{}, 2000.) == 2000. / 3);
	STATIC_ASSERT_IF_17(
			i.as_ref().map([](int k) { return 2.0 * k; }) == some(6.0));

	STATIC_ASSERT_IF_14(!option<int>{0}.and_then(A{}));
	STATIC_ASSERT_IF_14(option<int>{3}.and_then(A{}).unwrap() == 1000. / 3);
	STATIC_ASSERT_IF_14(option<int>{42}.take().unwrap() == 42);
	STATIC_ASSERT_IF_14(!option<int>{none}.take());
	STATIC_ASSERT_IF_14(!j.as_ref().and_then(A{}));

	STATIC_ASSERT_IF_14(option<int>{i}.or_else(A{}).unwrap() == 3);
	STATIC_ASSERT_IF_14(option<int>{j}.or_else(A{}).unwrap() == 13);

	STATIC_ASSERT(sizeof(option<int&>) == sizeof(int*));

	{
		__VEG_CPP14(constexpr)
		option<option<option<int>>> opt = {some, {some, {some, 3}}};
		__VEG_CPP14(constexpr)
		option<option<option<int>>> opt_also = some(some(some(3)));
		__VEG_CPP14(constexpr)
		option<option<option<int>>> opt2 = {some, {some, {none}}};
		STATIC_ASSERT_IF_14(opt == opt_also);
		STATIC_ASSERT_IF_14(
				opt //
						.as_ref()
						.unwrap()
						.as_ref()
						.unwrap()
						.as_ref()
						.unwrap() == 3);
		STATIC_ASSERT_IF_14(opt.clone().unwrap().unwrap().unwrap() == 3);
		STATIC_ASSERT_IF_14(opt.clone().flatten().flatten().unwrap() == 3);
		STATIC_ASSERT_IF_14(!opt2.clone().unwrap().unwrap());
		STATIC_ASSERT_IF_14(!opt2.clone().flatten().flatten());
		STATIC_ASSERT_IF_14(opt2.clone().flatten());
		STATIC_ASSERT_IF_14(opt2.clone().flatten());
	}

	{
		option<bool> flag;
		option<int> opt;
		opt.as_ref().map_or_else(
				[&](int /*unused*/) {
					flag = {some, true};
				},
				[&] {
					flag = {some, false};
				});
		ASSERT_TRUE(flag);
		ASSERT_FALSE(flag.as_ref().unwrap());

		opt = {some, 3};
		flag = none;
		opt.as_ref().map_or_else(
				[&](int /*unused*/) {
					flag = {some, true};
				},
				[&] {
					flag = {some, false};
				});
		ASSERT_TRUE(flag);
		ASSERT_TRUE(flag.as_ref().unwrap());
	}
	{
		__VEG_CPP17(constexpr)
		auto opt = [&] {
			option<int> x;
			x.emplace_with([&] { return 1; });
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(1));
	}
	{
		STATIC_ASSERT(__VEG_CONCEPT(meta::mostly_trivial<int>));
		STATIC_ASSERT(__VEG_CONCEPT(meta::mostly_trivial<option<int>>));
		__VEG_CPP17(constexpr)
		auto opt = [&] {
			option<option<int>> x;
			x.emplace_with([&] { return some(1); });
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(some(1)));
	}
	{
		using std::vector;

		auto opt = [&] {
			option<vector<int>> x;
			x.emplace_with([&](int i) { return vector<int>{1, 2, 3, i}; }, 5);
			return x;
		}();

		ASSERT_EQ(opt, some(vector<int>{1, 2, 3, 5}));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<vector<int>>>::value == 253);

		auto opt = [&] {
			option<option<vector<int>>> x;
			x.emplace_with([&] { return some(vector<int>{1, 2, 3}); });
			return x;
		}();

		ASSERT_EQ(opt, some(some(vector<int>{1, 2, 3})));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<option<vector<int>>>>::value == 252);

		auto opt = [&] {
			option<option<option<vector<int>>>> x;
			x.emplace_with([&] { return some(some(vector<int>{1, 2, 3})); });
			return x;
		}();

		ASSERT_EQ(opt, some(some(some(vector<int>{1, 2, 3}))));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<option<option<vector<int>>>>>::value ==
				251);

		auto opt = [&] {
			option<option<option<option<vector<int>>>>> x;
			x.emplace_with([&] { return some(some(some(vector<int>{1, 2, 3}))); });
			return x;
		}();

		ASSERT_EQ(opt, some(some(some(some(vector<int>{1, 2, 3})))));
	}

	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<vector<int>>>::value == 253);

		auto opt = [&] {
			option<option<vector<int>>> x;
			x.emplace(some(vector<int>{1, 2, 3}));
			return x;
		}();

		ASSERT_EQ(opt, some(some(vector<int>{1, 2, 3})));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<option<vector<int>>>>::value == 252);

		auto opt = [&] {
			option<option<option<vector<int>>>> x;
			x.emplace(some(some(vector<int>{1, 2, 3})));
			return x;
		}();

		ASSERT_EQ(opt, some(some(some(vector<int>{1, 2, 3}))));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				meta::value_sentinel_for<option<option<option<vector<int>>>>>::value ==
				251);

		auto opt = [&] {
			option<option<option<option<vector<int>>>>> x;
			x.emplace(some(some(some(vector<int>{1, 2, 3}))));
			return x;
		}();

		ASSERT_EQ(opt, some(some(some(some(vector<int>{1, 2, 3})))));
	}
}
