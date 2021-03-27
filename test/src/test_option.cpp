#include "veg/option.hpp"
#include "veg/fn_view.hpp"
#include <doctest.h>
#include <vector>
#include "static_assert.hpp"
#include "veg/internal/prologue.hpp"

using namespace veg;
template <typename T, i64 N>
struct nested_option {
	using type = option<typename nested_option<T, N - 1>::type>;
};
template <typename T>
struct nested_option<T, 0> {
	using type = T;
};

TEST_CASE("option: all") {

	using make::from_callable;
	struct A {
		VEG_CPP14(constexpr) auto operator()() const -> option<int> {
			return {some, 13};
		}
		VEG_CPP14(constexpr) auto operator()(int i) const -> option<double> {
			if (i == 0) {
				return none;
			}
			return {some, 1000. / i};
		}
	};

	struct B {
		VEG_CPP14(constexpr) auto operator()(int i) const -> double {
			if (i == 0) {
				return 1.0;
			}
			return 2000. / i;
		}
	};
	struct C {
		VEG_CPP14(constexpr) auto operator()() const -> double { return 2000.; }
	};

	STATIC_ASSERT(sizeof(option<bool>) == sizeof(bool) * 2);
	STATIC_ASSERT(sizeof(option<option<bool>>) == sizeof(bool) * 2);
	STATIC_ASSERT(sizeof(option<option<option<bool>>>) == sizeof(bool) * 2);

	STATIC_ASSERT(sizeof(option<int>) == sizeof(int) * 2);
	STATIC_ASSERT(sizeof(option<option<int>>) == sizeof(int) * 2);
	STATIC_ASSERT(sizeof(option<option<option<option<int>>>>) == sizeof(int) * 2);
	STATIC_ASSERT(
			sizeof(option<fn::fn_view<void()>>) == sizeof(fn::fn_view<void()>));
	STATIC_ASSERT(
			sizeof(option<fn::fn_view<void()>>) == sizeof(fn::fn_view<void()>));

	VEG_CPP14(constexpr) option<int> i = {some, 3};
	VEG_CPP14(constexpr) option<int> j = none;
	STATIC_ASSERT_IF_14(i.as_ref().and_then(A{}).unwrap() == 1000. / 3);
	STATIC_ASSERT_IF_17(
			option<int>{some, inplace, from_callable([&] { return 0; })});
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
		VEG_CPP14(constexpr)
		option<option<option<int>>> opt = {some, {some, {some, 3}}};
		VEG_CPP14(constexpr)
		option<option<option<int>>> opt_also = some(some(some(3)));
		VEG_CPP14(constexpr)
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
		CHECK(flag);
		CHECK(!flag.as_ref().unwrap());

		opt = {some, 3};
		flag = none;
		opt.as_ref().map_or_else(
				[&](int /*unused*/) {
					flag = {some, true};
				},
				[&] {
					flag = {some, false};
				});
		CHECK(flag);
		CHECK(flag.as_ref().unwrap());
	}
	{
		VEG_CPP17(constexpr)
		auto opt = [&] {
			option<int> x;
			x.emplace(from_callable([&] { return 1; }));
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(1));
	}
	{
		STATIC_ASSERT(VEG_CONCEPT(mostly_trivial<int>));
		STATIC_ASSERT(VEG_CONCEPT(mostly_trivial<option<int>>));
		VEG_CPP17(constexpr)
		auto opt = [&] {
			option<option<int>> x;
			x.emplace(from_callable([&] { return some(1); }));
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(some(1)));
	}
	{
		using std::vector;

		auto opt = [&] {
			option<vector<int>> x;
			x.emplace(from_callable(
					[&](int i_) {
						return vector<int>{1, 2, 3, i_};
					},
					5));
			return x;
		}();

		CHECK(opt == some(vector<int>{1, 2, 3, 5}));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<option<vector<int>>>::spare_representations ==
				253);

		auto opt = [&] {
			option<option<vector<int>>> x;
			x.emplace(from_callable([&] { return some(vector<int>{1, 2, 3}); }));
			return x;
		}();

		CHECK(opt == some(some(vector<int>{1, 2, 3})));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<
						option<option<vector<int>>>>::spare_representations == 252);

		auto opt = [&] {
			option<option<option<vector<int>>>> x;
			x.emplace(from_callable([&] {
				return some(some(vector<int>{1, 2, 3}));
			}));
			return x;
		}();

		CHECK(opt == some(some(some(vector<int>{1, 2, 3}))));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<
						option<option<option<vector<int>>>>>::spare_representations == 251);

		auto opt = [&] {
			option<option<option<option<vector<int>>>>> x;
			x.emplace(from_callable([&] {
				return some(some(some(vector<int>{1, 2, 3})));
			}));
			return x;
		}();

		CHECK(opt == some(some(some(some(vector<int>{1, 2, 3})))));
	}

	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<option<vector<int>>>::spare_representations ==
				253);

		auto opt = [&] {
			option<option<vector<int>>> x;
			x.emplace(some(vector<int>{1, 2, 3}));
			return x;
		}();

		CHECK(opt == some(some(vector<int>{1, 2, 3})));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<
						option<option<vector<int>>>>::spare_representations == 252);

		auto opt = [&] {
			option<option<option<vector<int>>>> x;
			x.emplace(some(some(vector<int>{1, 2, 3})));
			return x;
		}();

		CHECK(opt == some(some(some(vector<int>{1, 2, 3}))));
	}
	{
		using std::vector;

		STATIC_ASSERT_IF_14(
				+meta::tombstone_traits<
						option<option<option<vector<int>>>>>::spare_representations == 251);

		auto opt = [&] {
			option<option<option<option<vector<int>>>>> x;
			x.emplace(some(some(some(vector<int>{1, 2, 3}))));
			return x;
		}();

		CHECK(opt == some(some(some(some(vector<int>{1, 2, 3})))));
	}
}
#include "veg/internal/epilogue.hpp"
