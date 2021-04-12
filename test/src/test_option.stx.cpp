/**
 * @file option_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "static_assert.hpp"
#include "veg/option.hpp"
#include "assert_death.hpp"

#include <memory>
#include <numeric>
#include <vector>
#include <algorithm>

#include <doctest.h>
#include <string>
#include "veg/internal/prologue.hpp"

using std::vector;
using namespace veg; // NOLINT

// TODO(lamarrr) test against all methods

template <size_t ID>
struct MoveOnly {
	explicit MoveOnly(int /*unused*/) {}
	// no implicit defaul construction
	MoveOnly() {
		VEG_ASSERT("\t>> MoveOnly<" + std::to_string(ID) + ">::construct", false);
	}
	MoveOnly(MoveOnly const& /*unused*/) {
		VEG_ASSERT(
				"\t>> MoveOnly<" + std::to_string(ID) + ">::copy_construct called",
				false);
	}
	MoveOnly(MoveOnly&&) noexcept = default;
	auto operator=(MoveOnly const& /*unused*/) -> MoveOnly& { // NOLINT
		VEG_ASSERT(
				"\t>> MoveOnly<" + std::to_string(ID) + ">::copy_assign called", false);
		return *this;
	}
	auto operator=(MoveOnly&&) noexcept -> MoveOnly& = default;
	~MoveOnly() noexcept = default;

	void done() const {}

	auto operator==(MoveOnly const& /*unused*/) const noexcept -> bool {
		return true;
	}
	auto operator!=(MoveOnly const& /*unused*/) const noexcept -> bool {
		return false;
	}
};

template <size_t id>
auto make_mv() -> MoveOnly<id> {
	return MoveOnly<id>(id);
}

struct NotEq {};

STATIC_ASSERT(VEG_CONCEPT(swappable<MoveOnly<0>&, MoveOnly<0>&>));
STATIC_ASSERT(VEG_CONCEPT(equality_comparable_with<MoveOnly<0>, MoveOnly<0>>));
STATIC_ASSERT(VEG_CONCEPT(
		equality_comparable_with<Option<MoveOnly<0>>, Option<MoveOnly<0>>>));
STATIC_ASSERT(!VEG_CONCEPT(equality_comparable_with<NotEq, NotEq>));
STATIC_ASSERT(
		!VEG_CONCEPT(equality_comparable_with<Option<NotEq>, Option<NotEq>>));

struct FnMut {
	int call_times{};
	FnMut() = default;
	auto operator()(int&& x) -> int {
		call_times++;
		return x;
	}
	auto operator()(int&& x) const -> int { return x; }
};

struct FnConst {
	auto operator()(int&& x) const -> int { return x; }
};

TEST_CASE("OptionTest: Misc") {
	CHECK(Option<Option<int>>({some, some(899)}).unwrap().unwrap() == 899);
}

TEST_CASE("OptionTest: ObjectConstructionTest") {
	Option<int> a = none;
	auto b = some(89);
	CHECK_DEATH({ void(VEG_MOV(a).unwrap()); });
	CHECK_NO_DEATH({ void(VEG_MOV(b).unwrap()); });
	CHECK(some(89).unwrap() == 89);

	auto fn_a = []() -> Option<MoveOnly<0>> {
		return {some, make_mv<0>()}; // NOLINT
	};
	(void)fn_a();
	auto fn_b = []() -> Option<MoveOnly<1>> { return none; };
	(void)fn_b();

	auto d = fn_a();
	d = {some, make_mv<0>()};
	d = none;
	d = {some, make_mv<0>()};
}

TEST_CASE("OptionTest: CopyConstructionTest") {
	Option<int> a = none;
	Option<int> b = a;
	CHECK(a == b);

	Option<int> c = {some, 98};
	b = c;
	CHECK(b == c);
	CHECK(a != c);
	CHECK(a != b);

	Option<vector<int>> d = none;
	Option<vector<int>> e = d;

	CHECK(d == e);

	auto f = some(vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
	e = f;
	CHECK(e == f);
	CHECK(d != e);
	CHECK(d != f);
}

TEST_CASE("OptionTest: ObjectForwardingTest") {
	auto fn_a = []() -> Option<MoveOnly<0>> {
		return {some, make_mv<0>()}; // NOLINT
	};
	(void)(fn_a().unwrap());
	auto fn_b = []() -> Option<std::unique_ptr<int[]>> {
		return {some, std::unique_ptr<int[]>(new int[1024])};
	};
	(void)(fn_b().unwrap());

	auto g = some(vector<int>{1, 2, 3, 4, 5});

	g = {some, vector<int>{5, 6, 7, 8, 9}};

	CHECK(g == some(vector<int>{5, 6, 7, 8, 9}));

	g = none;

	CHECK(!g);

	g = {some, vector<int>{1, 2, 3, 4, 5}};

	CHECK(g == some(vector<int>{1, 2, 3, 4, 5}));

	g = none;

	CHECK(!g);
}

TEST_CASE("OptionTest: Equality") {
	CHECK(some(0));
	CHECK(some(90) == some(90));
	CHECK(some(90) != some(70));
	CHECK((Option<Option<int>>{some, none}));
	CHECK(none == none);
	CHECK(some(90) == some(90));
	CHECK(some(90) != some(70));
	CHECK(some(90) == some(90));
	CHECK(some(90) != some(20));
	CHECK(some(90));
	CHECK(!Option<int>{none});
	CHECK((Option<Option<int>>{some, Option<int>(none)}));

	CHECK((Option<Option<int>>{some, Option<int>{none}}));
	CHECK(some(90) == some(90));
	CHECK(some(70) != some(90));
	CHECK(some(90));
	CHECK(!Option<int>(none));
	CHECK(Option<Option<int>>({some, Option<int>(none)}));

	int const x = 909909;
	int y = 909909;

	CHECK(some_ref(x) == some(909909));
	CHECK(some_ref(y) == some(909909));

	CHECK(some(909909) == some_ref(y));
	CHECK(some(909909) == some_ref(y));

	CHECK(some(909909) == some_ref(x));
	CHECK(some(909909) == some_ref(y));
	CHECK(some(101101) != some_ref(x));
	CHECK(some(101101) != some_ref(y));

	CHECK(some_ref(x) == some(909909));
	CHECK(some_ref(y) == some(909909));
	CHECK(some_ref(x) != some(101101));
	CHECK(some_ref(y) != some(101101));
}

TEST_CASE("OptionTest: Contains") {
	CHECK(some(vector<int>{1, 2, 3, 4}).contains(vector<int>{1, 2, 3, 4}));
	CHECK(!some(vector<int>{1, 2, 3, 4}).contains(vector<int>{1, 2, 3, 4, 5}));

	CHECK(some(8).contains(8));
	CHECK(!some(8).contains(88));
}

TEST_CASE("OptionLifetimeTest: Contains") {
	CHECK_NO_DEATH({ (void)some(make_mv<0>()); });
	CHECK_NO_DEATH({ (void)Option<MoveOnly<1>>{none}.contains(make_mv<1>()); });
}

TEST_CASE("OptionTest: Exists") {
	auto const even = [](int const& x) { return x % 2 == 0; };

	auto const all_even = [&](vector<int> const& x) {
		return std::all_of(x.begin(), x.end(), even);
	};

	CHECK(some(8).filter(even));
	CHECK(!some(81).filter(even));

	CHECK(some(vector<int>{2, 4, 6, 8, 10}).filter(all_even));
	CHECK(!some(vector<int>{2, 4, 6, 9, 10}).filter(all_even));
}

TEST_CASE("OptionTest: AsConstRef") {
	auto const a = some(68);
	CHECK(a.as_cref().unwrap() == 68);

	Option<int> const b = none;
	CHECK(!b.as_cref());

	auto const c = some(vector<int>{1, 2, 3, 4});
	CHECK(c.as_cref().unwrap() == vector<int>{1, 2, 3, 4});

	Option<vector<int>> const d = none;
	CHECK(!d.as_cref());
}

TEST_CASE("OptionTest: AsRef") {
	auto a = some(68);
	a.as_ref().unwrap() = 99;
	CHECK(a == some(99));

	Option<int> b = none;
	CHECK(!b.as_ref());

	auto c = some(vector<int>{1, 2, 3, 4});
	c.as_ref().unwrap() = vector<int>{5, 6, 7, 8, 9, 10};
	CHECK(c == some(vector<int>{5, 6, 7, 8, 9, 10}));

	auto d = Option<vector<int>>(none);
	CHECK(!d.as_ref());
}

TEST_CASE("OptionLifeTimeTest: AsRef") {
	auto a = some(make_mv<0>());
	CHECK_NO_DEATH({ (void)a.as_ref().unwrap().done(); });

	auto b = Option<MoveOnly<1>>(none);
	CHECK_NO_DEATH({ (void)a.as_ref().unwrap().done(); });
	CHECK_NO_DEATH({
		auto b_ = b.as_ref();
		(void)b_;
	});
}

TEST_CASE("OptionTest: Unwrap") {
	CHECK(some(0).unwrap() == 0);
	CHECK_DEATH({ (void)Option<int>(none).unwrap(); });

	CHECK(
			some(vector<int>{1, 2, 3, 4, 5}).unwrap() == vector<int>{1, 2, 3, 4, 5});
	CHECK_DEATH({ (void)Option<vector<int>>(none).unwrap(); });
}

TEST_CASE("OptionLifetimeTest: Unwrap") {
	auto a = some(make_mv<0>());
	CHECK_NO_DEATH({ VEG_FWD(a).unwrap().done(); });
}

// TEST_CASE("OptionTest: UnwrapOr") {
//   EXPECT_EQ(some( 0).unwrap_or(90), 0);
//   EXPECT_EQ(option<int>(none).unwrap_or(90), 90);

//   EXPECT_EQ(
//       some( vector<int>{1, 2, 3, 4, 5}}).unwrap_or(vector<int>{6, 7,
//       8, 9, 10}), vector<int>{1, 2, 3, 4, 5});
//   EXPECT_EQ(
//       option<vector<int>>(none).unwrap_or(vector<int>{6, 7, 8, 9, 10}),
//       vector<int>{6, 7, 8, 9, 10});
// }

// TEST_CASE("OptionLifetimeTest: UnwrapOr") {
//  auto a = some( make_mv<0>()});
//  EXPECT_NO_THROW(VEG_FWD(a).unwrap_or(make_mv<0>()).done());

//  auto b = option<MoveOnly<1>>(none);
//  EXPECT_NO_THROW(VEG_FWD(b).unwrap_or(make_mv<1>()).done());
//}

// TEST_CASE("OptionTest: UnwrapOrElse") {
//  auto&& a = some( 0}).unwrap_or_else([]() { return 90; });
//  EXPECT_EQ(a, 0);
//  auto&& b = option<int>(none).unwrap_or_else([]() { return 90; });
//  EXPECT_EQ(b, 90);

//  auto&& c = some( vector<int>{1, 2, 3, 4, 5}}).unwrap_or_else([]() {
//    return vector<int>{6, 7, 8, 9, 10};
//  });
//  EXPECT_EQ(c, vector<int>{1, 2, 3, 4, 5});

//  auto&& d = option<vector<int>>(none).unwrap_or_else([]() {
//    return vector<int>{6, 7, 8, 9, 10};
//  });
//  EXPECT_EQ(d, vector<int>{6, 7, 8, 9, 10});
//}

// TEST_CASE("OptionLifetimeTest: UnwrapOrElse") {
//  auto a = some( make_mv<0>()});
//  auto fn = []() { return make_mv<0>(); };
//  EXPECT_NO_THROW(VEG_FWD(a).unwrap_or_else(fn).done());

//  auto b = option<MoveOnly<1>>(none);
//  auto fn_b = []() { return make_mv<1>(); };
//  EXPECT_NO_THROW(VEG_FWD(b).unwrap_or_else(fn_b).done());
//}

// TEST_CASE("OptionTest: Map") {
//  auto&& a = some( 90}).map([](int&& x) -> int { return x + 90; });
//  EXPECT_EQ(a, {some, 180});

//  auto&& b = option<int>(none).map([](int&& x) { return x + 90; });
//  EXPECT_EQ(b, none);

//  auto&& c = some( vector<int>{1, 2, 3, 4, 5}}).map([](vector<int>&&
//  vec) {
//    vec.push_back(6);
//    return VEG_FWD(vec);
//  });
//  EXPECT_EQ(c, {some, vector<int>{1, 2, 3, 4, 5, 6}});

//  auto&& d = option<vector<int>>(none).map([](vector<int>&& vec) {
//    vec.push_back(6);
//    return VEG_FWD(vec);
//  });
//  EXPECT_EQ(d, none);
//}

TEST_CASE("OptionLifetimeTest: Map") {
	auto a = some(make_mv<0>());
	CHECK_NO_DEATH(
			{ VEG_FWD(a).map([](MoveOnly<0> r) { return r; }).unwrap().done(); });
}

TEST_CASE("OptionTest: FnMutMap") {
	auto fnmut_a = FnMut();
	auto a1_ = some(90).map(fnmut_a);
	auto a2_ = some(90).map(fnmut_a);

	CHECK(fnmut_a.call_times == 2);

	auto const fnmut_b = FnMut();
	auto b1_ = some(90).map(fnmut_b);
	auto b2_ = some(90).map(fnmut_b);
	CHECK(fnmut_b.call_times == 0);

	auto fnconst = FnConst();
	auto c = some(90).map(fnconst);

	(void)a1_, (void)a2_, (void)b1_, (void)b2_, (void)c;
}

TEST_CASE("OptionTest: MapOrElse") {
	auto&& a = some(90).map_or_else(
			[](int&& x) -> int { return x + 90; }, []() -> int { return 90; });
	CHECK(a == 180);

	auto&& b = Option<int>(none).map_or_else(
			[](int&& x) -> int { return x + 90; }, []() -> int { return 90; });
	CHECK(b == 90);
}

TEST_CASE("OptionLifetimeTest: MapOrElse") {
	auto a = some(make_mv<0>());
	auto fn = [](MoveOnly<0>) { return make_mv<0>(); };
	auto fn_b = []() { return make_mv<0>(); };
	CHECK_NO_DEATH({ VEG_FWD(a).map_or_else(fn, fn_b).done(); });
}

// TEST_CASE("OptionTest: And") {
//  auto&& a = some( 90}).AND(some( 90.0f}));

//  EXPECT_FLOAT_EQ(VEG_FWD(a).unwrap(), 90.0f);

//  auto&& b = make_none<int>().AND(some( 90.0f}));
//  EXPECT_EQ(b, none);

//  auto c = []() {
//    return some( 90})
//        .AND(some( vector<float>{90.0f, 180.0f, 3.141f}}));
//  };

//  EXPECT_EQ(c().unwrap(), vector<float>{90.0f, 180.0f, 3.141f});

//  auto&& d =
//      make_none<int>().AND(some( vector<float>{90.0f,
//      180.0f, 3.141f}}));
//  EXPECT_EQ(d, none);
//}

// TEST_CASE("OptionLifetimeTest: And") {
//  EXPECT_NO_THROW(some( make_mv<0>()})
//                      .AND(some( make_mv<1>()}))
//                      .unwrap()
//                      .done());
//  EXPECT_EQ(make_none<int>().AND(some( make_mv<2>()})), none);
//}

// TEST_CASE("OptionTest: AndThen") {
//  auto&& a = some( 90}).and_then([](int&& x) {
//    return some( static_cast<float>(x) + 90.0f});
//  });

//  EXPECT_FLOAT_EQ(VEG_FWD(a).unwrap(), 180.0f);

//  auto&& b = make_none<int>().and_then([](int&& x) {
//    return some( static_cast<float>(x) + 90.0f});
//  });
//  EXPECT_EQ(b, none);

//  //
//}

TEST_CASE("OptionTest: Filter") {
	auto is_even = [](int const& num) { return num % 2 == 0; };
	auto is_odd = [&](int const& num) { return !(is_even(num)); };

	CHECK(some(90).filter(is_even).unwrap() == 90);
	CHECK(some(99).filter(is_odd).unwrap() == 99);

	CHECK(!Option<int>(none).filter(is_even));
	CHECK(!Option<int>(none).filter(is_even));

	auto all_odd = [&](vector<int> const& vec) {
		return all_of(vec.begin(), vec.end(), is_odd);
	};

	CHECK(!some(vector<int>{1, 3, 5, 7, 2, 4, 6, 8}).filter(all_odd));
	CHECK(
			some(vector<int>{1, 3, 5, 7}).filter(all_odd).unwrap() ==
			vector<int>{1, 3, 5, 7});

	CHECK(!Option<vector<int>>(none).filter(all_odd));
	CHECK(!Option<vector<int>>(none).filter(all_odd));
}

// TEST_CASE("OptionTest: FilterNot") {
//  auto is_even = [](int const& num) { return num % 2 == 0; };

//  EXPECT_EQ(some( 90}).filter_not(is_even), none);
//  EXPECT_EQ(some( 99}).filter_not(is_even), {some, 99});

//  EXPECT_EQ(make_none<int>().filter_not(is_even), none);
//  EXPECT_EQ(make_none<int>().filter_not(is_even), none);

//  auto all_odd = [&](vector<int> const& vec) {
//    return all_of(vec.begin(), vec.end(), [=](auto x) { return !is_even(x);
//    });
//  };

//  EXPECT_EQ(
//      some( vector<int>{1, 3, 5, 7, 2, 4, 6, 8}}).filter_not(all_odd),
//      {some, vector<int>{1, 3, 5, 7, 2, 4, 6, 8}});
//  EXPECT_EQ(make_some(vector<int>{1, 3, 5, 7}).filter_not(all_odd), none);

//  EXPECT_EQ(make_none<vector<int>>().filter_not(all_odd), none);
//  EXPECT_EQ(make_none<vector<int>>().filter_not(all_odd), none);
//}

// TEST_CASE("OptionTest: Or") {
//  auto&& a = some( 90}).OR(some( 89}));
//  EXPECT_EQ(VEG_FWD(a).unwrap(), 90);

//  auto&& b = make_none<int>().OR(some( 89}));
//  EXPECT_EQ(VEG_FWD(b).unwrap(), 89);

//  auto&& c = make_none<int>().OR(make_none<int>());
//  EXPECT_EQ(c, none);
//  //
//  //
//  auto&& d = some( vector<int>{1, 2, 3, 4, 5}})
//                 .OR(some( vector<int>{6, 7, 8, 9, 10}}));
//  EXPECT_EQ(VEG_FWD(d).unwrap(), vector<int>{1, 2, 3, 4, 5});

//  auto&& e =
//      option<vector<int>>(none).OR(some( vector<int>{6, 7, 8, 9,
//      10}}));
//  EXPECT_EQ(VEG_FWD(e).unwrap(), vector<int>{6, 7, 8, 9, 10});

//  auto&& f = option<vector<int>>(none).OR(option<vector<int>>(none));
//  EXPECT_EQ(f, none);
//}

// TEST_CASE("OptionTest: Xor") {
//  auto&& a = some( 90}).XOR(some( 89}));
//  EXPECT_EQ(a, none);

//  auto&& b = make_none<int>().XOR(some( 89}));
//  EXPECT_EQ(VEG_FWD(b).unwrap(), 89);

//  auto&& c = make_none<int>().XOR(make_none<int>());
//  EXPECT_EQ(c, none);
//  //
//  //
//  auto&& d = some( vector<int>{1, 2, 3, 4, 5}})
//                 .XOR(some( vector<int>{6, 7, 8, 9, 10}}));
//  EXPECT_EQ(d, none);

//  auto&& e =
//      option<vector<int>>(none).XOR(some( vector<int>{6, 7, 8, 9,
//      10}}));
//  EXPECT_EQ(VEG_FWD(e).unwrap(), vector<int>{6, 7, 8, 9, 10});

//  auto&& f = option<vector<int>>(none).XOR(option<vector<int>>(none));
//  EXPECT_EQ(f, none);
//}

TEST_CASE("OptionTest: Take") {
	auto a = some(9);
	CHECK(a.take().unwrap() == 9);
	CHECK(!a);

	auto b = Option<int>(none);
	CHECK(!b.take());
	CHECK(!b);

	auto c = some(vector<int>{-1, -2, -4, -8, -16});
	auto ca = c.take();
	CHECK(ca == some(vector<int>{-1, -2, -4, -8, -16}));
	CHECK(!c);

	auto d = Option<vector<int>>(none);
	CHECK(!d.take());
	CHECK(!d);
}

// TEST_CASE("OptionTest: Replace") {
//  auto a = some( 9});
//  EXPECT_EQ(a.replace(27), {some, 9});
//  EXPECT_EQ(a, {some, 27});

//  auto b = option<int>(none);
//  EXPECT_EQ(b.replace(88), none);
//  EXPECT_EQ(b, {some, 88});

//  auto c = some( vector<int>{-1, -2, -4, -8, -16}});
//  EXPECT_EQ(c.replace(vector<int>{}), {some, vector<int>{-1, -2, -4, -8,
//  -16}}); EXPECT_EQ(c, {some, vector<int>{}});

//  auto d = option<vector<int>>(none);
//  EXPECT_EQ(d.replace(vector<int>{1, 2, 3, 4, 5}), none);
//  EXPECT_EQ(d, {some, vector<int>{1, 2, 3, 4, 5}});
//}

TEST_CASE("OptionTest: Clone") {
	auto a = some(9);
	CHECK(a.clone() == some(9));
	CHECK(a == some(9));

	auto b = some(static_cast<int*>(nullptr));
	CHECK(b.clone() == some(static_cast<int*>(nullptr)));
	CHECK(b == some(static_cast<int*>(nullptr)));

	auto c = some(vector<int>{1, 2, 3, 4, 5});
	CHECK(c.clone() == some(vector<int>{1, 2, 3, 4, 5}));
	CHECK(c == some(vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("OptionTest: OrElse") {
	auto&& a = some(90.0F).or_else([]() { return some(0.5f); });
	CHECK(VEG_FWD(a).unwrap() == 90.0F);

	auto&& b = Option<float>(none).or_else([]() { return some(0.5f); });
	CHECK(VEG_FWD(b).unwrap() == 0.5F);

	auto&& c = Option<float>(none).or_else([]() { return Option<float>(none); });
	CHECK(!c);
	//
	//
	auto&& d = some(vector<int>{1, 2, 3, 4, 5}).or_else([]() {
		return some(vector<int>{6, 7, 8, 9, 10});
	});
	CHECK(VEG_FWD(d).unwrap() == vector<int>{1, 2, 3, 4, 5});

	auto&& e = Option<vector<int>>(none).or_else([]() {
		return some(vector<int>{6, 7, 8, 9, 10});
	});
	CHECK(VEG_FWD(e).unwrap() == vector<int>{6, 7, 8, 9, 10});

	auto&& f = Option<vector<int>>(none).or_else(
			[]() { return Option<vector<int>>(none); });
	CHECK(!f);
}
#include "veg/internal/epilogue.hpp"
