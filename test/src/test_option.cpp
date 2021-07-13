#include "veg/option.hpp"
#include "veg/fn_ref.hpp"
#include <doctest.h>
#include <vector>
#include "static_assert.hpp"
#include "veg/internal/prologue.hpp"

using namespace veg;
template <typename T, i64 N>
struct nested_option {
	using type = Option<typename nested_option<T, N - 1>::type>;
};
template <typename T>
struct nested_option<T, 0> {
	using type = T;
};

TEST_CASE("option: all") {

	struct A {
		VEG_CPP14(constexpr) auto operator()() const -> Option<int> {
			return {some, 13};
		}
		VEG_CPP14(constexpr) auto operator()(Ref<int> i) const -> Option<double> {
			if (*i == 0) {
				return none;
			}
			return {some, 1000. / *i};
		}
	};

	struct B {
		VEG_CPP14(constexpr) auto operator()(Ref<int> i) const -> double {
			if (*i == 0) {
				return 1.0;
			}
			return 2000. / *i;
		}
	};
	struct C {
		VEG_CPP14(constexpr) auto operator()() const -> double { return 2000.; }
	};

	constexpr Option<int> i = {some, 3};
	constexpr Option<int> j = none;
	STATIC_ASSERT_IF_14(i.as_ref().unwrap().get() == 3);
	STATIC_ASSERT_IF_14(i.as_ref().and_then(A{}).is_some());
	STATIC_ASSERT_IF_14(i.as_ref().and_then(A{}).unwrap() == 1000. / 3);
	STATIC_ASSERT_IF_17(Option<int>{inplace, [&] { return 0; }}.is_some());
	STATIC_ASSERT_IF_14(i.as_ref().map(B{}).unwrap() == 2000. / 3);

	STATIC_ASSERT_IF_14(i.as_ref().map_or_else(B{}, C{}) == 2000. / 3);
	STATIC_ASSERT_IF_14(j.as_ref().map_or_else(B{}, C{}) == 2000.);
	STATIC_ASSERT_IF_14(j.as_ref().map_or(B{}, 2000.) == 2000.);
	STATIC_ASSERT_IF_14(i.as_ref().map_or(B{}, 2000.) == 2000. / 3);
	STATIC_ASSERT_IF_17(
			bool(i.as_ref().map([](Ref<int> k) { return 2.0 * *k; }) == some(6.0)));

	STATIC_ASSERT_IF_14(Option<int>{some, 0}.as_ref().and_then(A{}).is_none());
	STATIC_ASSERT_IF_14(
			Option<int>{some, 3}.as_ref().and_then(A{}).unwrap() == 1000. / 3);
	STATIC_ASSERT_IF_14(Option<int>{some, 42}.take().unwrap() == 42);
	STATIC_ASSERT_IF_14(Option<int>{none}.take().is_none());
	STATIC_ASSERT_IF_14(j.as_ref().and_then(A{}).is_none());

	STATIC_ASSERT_IF_14(Option<int>{i}.or_else(A{}).unwrap() == 3);
	STATIC_ASSERT_IF_14(Option<int>{j}.or_else(A{}).unwrap() == 13);

	STATIC_ASSERT(sizeof(Option<int&>) == sizeof(int*));

	{
		using veg::clone;
		VEG_CPP14(constexpr)
		Option<Option<Option<int>>> opt = some(some(some(3)));
		VEG_CPP14(constexpr)
		Option<Option<Option<int>>> opt_also = some(some(some(3)));
		VEG_CPP14(constexpr)
		Option<Option<Option<int>>> opt2 = some(some(Option<int>(none)));
		STATIC_ASSERT_IF_14(opt == opt_also);
		STATIC_ASSERT_IF_14(
				*opt //
						 .as_ref()
						 .unwrap()
						 ->as_ref()
						 .unwrap()
						 ->as_ref()
						 .unwrap() == 3);
		STATIC_ASSERT_IF_14(clone(opt).unwrap().unwrap().unwrap() == 3);
		STATIC_ASSERT_IF_14(clone(opt).flatten().flatten().unwrap() == 3);
		STATIC_ASSERT_IF_14(clone(opt2).unwrap().unwrap().is_none());
		STATIC_ASSERT_IF_14(clone(opt2).flatten().flatten().is_none());
		STATIC_ASSERT_IF_14(clone(opt2).flatten().is_some());
		STATIC_ASSERT_IF_14(clone(opt2).flatten().is_some());
	}

	{
		Option<bool> flag;
		Option<int> opt;
		opt.as_ref().map_or_else(
				[&](Ref<int> /*unused*/) {
					flag = {some, true};
				},
				[&] {
					flag = {some, false};
				});
		CHECK(flag.is_some());
		CHECK(!*flag.as_ref().unwrap());

		opt = {some, 3};
		flag = none;
		opt.as_ref().map_or_else(
				[&](Ref<int> /*unused*/) {
					flag = {some, true};
				},
				[&] {
					flag = {some, false};
				});
		CHECK(flag.is_some());
		CHECK(*flag.as_ref().unwrap());
	}
	{
		VEG_CPP17(constexpr)
		auto opt = [&] {
			Option<int> x;
			x.emplace([&] { return 1; });
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(1));
	}
	{
		STATIC_ASSERT(veg::cpo::is_trivially_constructible<int>::value);
		STATIC_ASSERT(veg::cpo::is_trivially_constructible<Option<int>>::value);
		VEG_CPP17(constexpr)
		auto opt = [&] {
			Option<Option<int>> x;
			x.emplace([&] { return some(1); });
			return x;
		}();

		STATIC_ASSERT_IF_17(opt == some(some(1)));
	}
	{
		using std::vector;

		auto opt = [&] {
			Option<vector<int>> x;
			x.emplace([&] { return vector<int>{1, 2, 3, 4}; });
			return x;
		}();

		CHECK(opt == some(vector<int>{1, 2, 3, 4}));
		CHECK(VEG_FWD(opt).map([](vector<int> v) {
			v.push_back(5);
			return v;
		}) == some(vector<int>{1, 2, 3, 4, 5}));
	}
}
#include "veg/internal/epilogue.hpp"
