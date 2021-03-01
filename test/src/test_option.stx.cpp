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

#include <memory>
#include <numeric>
#include <vector>
#include <algorithm>

#include <gtest/gtest.h>

using namespace std; // NOLINT
using namespace veg; // NOLINT

// TODO(lamarrr) test against all methods

template <size_t ID>
struct MoveOnly {
  explicit MoveOnly(int /*unused*/) {}
  // no implicit defaul construction
  MoveOnly() {
    VEG_ASSERT("\t>> MoveOnly<" + to_string(ID) + ">::construct", false);
  }
  MoveOnly(MoveOnly const& /*unused*/) {
    VEG_ASSERT(
        "\t>> MoveOnly<" + to_string(ID) + ">::copy_construct called", false);
  }
  MoveOnly(MoveOnly&&) noexcept = default;
  auto operator=(MoveOnly const& /*unused*/) -> MoveOnly& { // NOLINT
    VEG_ASSERT(
        "\t>> MoveOnly<" + to_string(ID) + ">::copy_assign called", false);
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

STATIC_ASSERT(__VEG_CONCEPT(meta::swappable<MoveOnly<0>&, MoveOnly<0>&>));
STATIC_ASSERT(
    __VEG_CONCEPT(meta::equality_comparable_with<MoveOnly<0>, MoveOnly<0>>));
STATIC_ASSERT(__VEG_CONCEPT(
    meta::equality_comparable_with<option<MoveOnly<0>>, option<MoveOnly<0>>>));
STATIC_ASSERT(!__VEG_CONCEPT(meta::equality_comparable_with<NotEq, NotEq>));
STATIC_ASSERT(!__VEG_CONCEPT(
    meta::equality_comparable_with<option<NotEq>, option<NotEq>>));

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

TEST(OptionTest, Misc) {
  EXPECT_EQ(option<option<int>>({some, {some, 899}}).unwrap().unwrap(), 899);
}

TEST(OptionTest, ObjectConstructionTest) {
  option<int> a = none;
  auto b = some(89);
  EXPECT_DEATH(void(VEG_MOV(a).unwrap()), ".*");
  EXPECT_TRUE((void(VEG_MOV(b).unwrap()), true));
  EXPECT_EQ(some(89).unwrap(), 89);

  auto fn_a = []() -> option<MoveOnly<0>> {
    return {some, make_mv<0>()}; // NOLINT
  };
  (void)fn_a();
  auto fn_b = []() -> option<MoveOnly<1>> { return none; };
  (void)fn_b();

  auto d = fn_a();
  d = {some, make_mv<0>()};
  d = none;
  d = {some, make_mv<0>()};
}

TEST(OptionTest, CopyConstructionTest) {
  option<int> a = none;
  option<int> b = a;
  EXPECT_EQ(a, b);

  option<int> c = {some, 98};
  b = c;
  EXPECT_EQ(b, c);
  EXPECT_NE(a, c);
  EXPECT_NE(a, b);

  option<vector<int>> d = none;
  option<vector<int>> e = d;

  EXPECT_EQ(d, e);

  auto f = some(vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  e = f;
  EXPECT_EQ(e, f);
  EXPECT_NE(d, e);
  EXPECT_NE(d, f);
}

TEST(OptionTest, ObjectForwardingTest) {
  auto fn_a = []() -> option<MoveOnly<0>> {
    return {some, make_mv<0>()}; // NOLINT
  };
  (void)(fn_a().unwrap());
  auto fn_b = []() -> option<unique_ptr<int[]>> {
    return {some, std::unique_ptr<int[]>(new int[1024])};
  };
  (void)(fn_b().unwrap());

  auto g = some(vector<int>{1, 2, 3, 4, 5});

  g = {some, vector<int>{5, 6, 7, 8, 9}};

  EXPECT_EQ(g, some(vector<int>{5, 6, 7, 8, 9}));

  g = none;

  EXPECT_EQ(g, none);

  g = {some, vector<int>{1, 2, 3, 4, 5}};

  EXPECT_EQ(g, some(vector<int>{1, 2, 3, 4, 5}));

  g = none;

  EXPECT_EQ(g, none);
}

TEST(OptionTest, Equality) {
  EXPECT_NE((some(0)), none);
  EXPECT_EQ((some(90)), (some(90)));
  EXPECT_NE((some(90)), (some(70)));
  EXPECT_NE((option<option<int>>{some, none}), none);
  EXPECT_EQ(none, none);
  EXPECT_EQ((some(90)), (some(90)));
  EXPECT_NE((some(90)), (some(70)));
  EXPECT_EQ((some(90)), (some(90)));
  EXPECT_NE((some(90)), (some(20)));
  EXPECT_NE((some(90)), none);
  EXPECT_EQ(option<int>{none}, none);
  EXPECT_NE((option<option<int>>{some, option<int>(none)}), none);

  EXPECT_NE(none, (some(0)));
  EXPECT_NE((option<option<int>>{some, option<int>{none}}), none);
  EXPECT_EQ((some(90)), (some(90)));
  EXPECT_NE((some(70)), (some(90)));
  EXPECT_NE(none, (some(90)));
  EXPECT_EQ(none, option<int>(none));
  EXPECT_NE(none, option<option<int>>({some, option<int>(none)}));

  int const x = 909909;
  int y = 909909;

  EXPECT_EQ(some_ref(x), (some(909909)));
  EXPECT_EQ((some_ref(y)), (some(909909)));

  EXPECT_EQ((some(909909)), some_ref(y));
  EXPECT_EQ((some(909909)), some_ref(y));

  EXPECT_EQ((some(909909)), (some_ref(x)));
  EXPECT_EQ((some(909909)), (some_ref(y)));
  EXPECT_NE((some(101101)), (some_ref(x)));
  EXPECT_NE((some(101101)), (some_ref(y)));

  EXPECT_EQ(some_ref(x), (some(909909)));
  EXPECT_EQ(some_ref(y), (some(909909)));
  EXPECT_NE(some_ref(x), (some(101101)));
  EXPECT_NE(some_ref(y), (some(101101)));
}

TEST(OptionTest, Contains) {
  EXPECT_TRUE(some(vector<int>{1, 2, 3, 4}).contains(vector<int>{1, 2, 3, 4}));
  EXPECT_FALSE(
      some(vector<int>{1, 2, 3, 4}).contains(vector<int>{1, 2, 3, 4, 5}));

  EXPECT_TRUE(some(8).contains(8));
  EXPECT_FALSE(some(8).contains(88));
}

#define EXPECT_NO_DEATH(...)                                                   \
  EXPECT_EXIT(                                                                 \
      [&] { __VA_ARGS__ ::std::exit(0); }(),                                   \
      ::testing::ExitedWithCode(0),                                            \
      ".*")

TEST(OptionLifetimeTest, Contains) {
  EXPECT_NO_DEATH((void)some(make_mv<0>()););
  EXPECT_NO_DEATH((void)option<MoveOnly<1>>{none}.contains(make_mv<1>()););
}

TEST(OptionTest, Exists) {
  auto const even = [](int const& x) { return x % 2 == 0; };

  auto const all_even = [&](vector<int> const& x) {
    return std::all_of(x.begin(), x.end(), even);
  };

  EXPECT_TRUE(some(8).filter(even));
  EXPECT_FALSE(some(81).filter(even));

  EXPECT_TRUE(some(vector<int>{2, 4, 6, 8, 10}).filter(all_even));
  EXPECT_FALSE(some(vector<int>{2, 4, 6, 9, 10}).filter(all_even));
}

TEST(OptionTest, AsConstRef) {
  auto const a = some(68);
  EXPECT_EQ(a.as_cref().unwrap(), 68);

  option<int> const b = none;
  EXPECT_EQ(b.as_cref(), none);

  auto const c = some(vector<int>{1, 2, 3, 4});
  EXPECT_EQ(c.as_cref().unwrap(), (vector<int>{1, 2, 3, 4}));

  option<vector<int>> const d = none;
  EXPECT_EQ(d.as_cref(), none);
}

TEST(OptionTest, AsRef) {
  auto a = some(68);
  a.as_ref().unwrap() = 99;
  EXPECT_EQ(a, some(99));

  option<int> b = none;
  EXPECT_EQ(b.as_ref(), none);

  auto c = some(vector<int>{1, 2, 3, 4});
  c.as_ref().unwrap() = vector<int>{5, 6, 7, 8, 9, 10};
  EXPECT_EQ(c, some(vector<int>{5, 6, 7, 8, 9, 10}));

  auto d = option<vector<int>>(none);
  EXPECT_EQ(d.as_ref(), none);
}

TEST(OptionLifeTimeTest, AsRef) {
  auto a = some(make_mv<0>());
  EXPECT_NO_DEATH((void)a.as_ref().unwrap().done(););

  auto b = option<MoveOnly<1>>(none);
  EXPECT_NO_DEATH((void)a.as_ref().unwrap().done(););
  EXPECT_NO_DEATH(auto b_ = b.as_ref(); (void)b_;);
}

TEST(OptionTest, Unwrap) {
  EXPECT_EQ((some(0)).unwrap(), 0);
  EXPECT_DEATH((void)option<int>(none).unwrap(), ".*");

  EXPECT_EQ(
      (some(vector<int>{1, 2, 3, 4, 5})).unwrap(),
      (vector<int>{1, 2, 3, 4, 5}));
  EXPECT_DEATH((void)option<vector<int>>(none).unwrap(), ".*");
}

TEST(OptionLifetimeTest, Unwrap) {
  auto a = some(make_mv<0>());
  EXPECT_NO_DEATH(move(a).unwrap().done(););
}

// TEST(OptionTest, UnwrapOr) {
//   EXPECT_EQ((some( 0)).unwrap_or(90), 0);
//   EXPECT_EQ(option<int>(none).unwrap_or(90), 90);

//   EXPECT_EQ(
//       some( vector<int>{1, 2, 3, 4, 5}}).unwrap_or(vector<int>{6, 7,
//       8, 9, 10}), (vector<int>{1, 2, 3, 4, 5}));
//   EXPECT_EQ(
//       option<vector<int>>(none).unwrap_or(vector<int>{6, 7, 8, 9, 10}),
//       (vector<int>{6, 7, 8, 9, 10}));
// }

// TEST(OptionLifetimeTest, UnwrapOr) {
//  auto a = some( make_mv<0>()});
//  EXPECT_NO_THROW(move(a).unwrap_or(make_mv<0>()).done());

//  auto b = option<MoveOnly<1>>(none);
//  EXPECT_NO_THROW(move(b).unwrap_or(make_mv<1>()).done());
//}

// TEST(OptionTest, UnwrapOrElse) {
//  auto&& a = some( 0}).unwrap_or_else([]() { return 90; });
//  EXPECT_EQ(a, 0);
//  auto&& b = option<int>(none).unwrap_or_else([]() { return 90; });
//  EXPECT_EQ(b, 90);

//  auto&& c = some( vector<int>{1, 2, 3, 4, 5}}).unwrap_or_else([]() {
//    return vector<int>{6, 7, 8, 9, 10};
//  });
//  EXPECT_EQ(c, (vector<int>{1, 2, 3, 4, 5}));

//  auto&& d = option<vector<int>>(none).unwrap_or_else([]() {
//    return vector<int>{6, 7, 8, 9, 10};
//  });
//  EXPECT_EQ(d, (vector<int>{6, 7, 8, 9, 10}));
//}

// TEST(OptionLifetimeTest, UnwrapOrElse) {
//  auto a = some( make_mv<0>()});
//  auto fn = []() { return make_mv<0>(); };
//  EXPECT_NO_THROW(move(a).unwrap_or_else(fn).done());

//  auto b = option<MoveOnly<1>>(none);
//  auto fn_b = []() { return make_mv<1>(); };
//  EXPECT_NO_THROW(move(b).unwrap_or_else(fn_b).done());
//}

// TEST(OptionTest, Map) {
//  auto&& a = some( 90}).map([](int&& x) -> int { return x + 90; });
//  EXPECT_EQ(a, {some, 180});

//  auto&& b = option<int>(none).map([](int&& x) { return x + 90; });
//  EXPECT_EQ(b, none);

//  auto&& c = some( vector<int>{1, 2, 3, 4, 5}}).map([](vector<int>&&
//  vec) {
//    vec.push_back(6);
//    return move(vec);
//  });
//  EXPECT_EQ(c, {some, vector<int>{1, 2, 3, 4, 5, 6}});

//  auto&& d = option<vector<int>>(none).map([](vector<int>&& vec) {
//    vec.push_back(6);
//    return move(vec);
//  });
//  EXPECT_EQ(d, none);
//}

TEST(OptionLifetimeTest, Map) {
  auto a = some(make_mv<0>());
  EXPECT_NO_DEATH(
      move(a).map([](MoveOnly<0> r) { return r; }).unwrap().done(););
}

TEST(OptionTest, FnMutMap) {
  auto fnmut_a = FnMut();
  auto a1_ = some(90).map(fnmut_a);
  auto a2_ = some(90).map(fnmut_a);

  EXPECT_EQ(fnmut_a.call_times, 2);

  auto const fnmut_b = FnMut();
  auto b1_ = some(90).map(fnmut_b);
  auto b2_ = some(90).map(fnmut_b);
  EXPECT_EQ(fnmut_b.call_times, 0);

  auto fnconst = FnConst();
  auto c = some(90).map(fnconst);

  (void)a1_, (void)a2_, (void)b1_, (void)b2_, (void)c;
}

TEST(OptionTest, MapOrElse) {
  auto&& a = some(90).map_or_else(
      [](int&& x) -> int { return x + 90; }, []() -> int { return 90; });
  EXPECT_EQ(a, 180);

  auto&& b = option<int>(none).map_or_else(
      [](int&& x) -> int { return x + 90; }, []() -> int { return 90; });
  EXPECT_EQ(b, 90);
}

TEST(OptionLifetimeTest, MapOrElse) {
  auto a = some(make_mv<0>());
  auto fn = [](MoveOnly<0>) { return make_mv<0>(); };
  auto fn_b = []() { return make_mv<0>(); };
  EXPECT_NO_DEATH(move(a).map_or_else(fn, fn_b).done(););
}

// TEST(OptionTest, And) {
//  auto&& a = some( 90}).AND(some( 90.0f}));

//  EXPECT_FLOAT_EQ(move(a).unwrap(), 90.0f);

//  auto&& b = make_none<int>().AND(some( 90.0f}));
//  EXPECT_EQ(b, none);

//  auto c = []() {
//    return some( 90})
//        .AND(some( vector<float>{90.0f, 180.0f, 3.141f}}));
//  };

//  EXPECT_EQ(c().unwrap(), (vector<float>{90.0f, 180.0f, 3.141f}));

//  auto&& d =
//      make_none<int>().AND(some( vector<float>{90.0f,
//      180.0f, 3.141f}}));
//  EXPECT_EQ(d, none);
//}

// TEST(OptionLifetimeTest, And) {
//  EXPECT_NO_THROW(some( make_mv<0>()})
//                      .AND(some( make_mv<1>()}))
//                      .unwrap()
//                      .done());
//  EXPECT_EQ(make_none<int>().AND(some( make_mv<2>()})), none);
//}

// TEST(OptionTest, AndThen) {
//  auto&& a = some( 90}).and_then([](int&& x) {
//    return some( static_cast<float>(x) + 90.0f});
//  });

//  EXPECT_FLOAT_EQ(move(a).unwrap(), 180.0f);

//  auto&& b = make_none<int>().and_then([](int&& x) {
//    return some( static_cast<float>(x) + 90.0f});
//  });
//  EXPECT_EQ(b, none);

//  //
//}

TEST(OptionTest, Filter) {
  auto is_even = [](int const& num) { return num % 2 == 0; };
  auto is_odd = [&](int const& num) { return !(is_even(num)); };

  EXPECT_EQ((some(90).filter(is_even).unwrap()), 90);
  EXPECT_EQ((some(99).filter(is_odd).unwrap()), 99);

  EXPECT_EQ(option<int>(none).filter(is_even), none);
  EXPECT_EQ(option<int>(none).filter(is_even), none);

  auto all_odd = [&](vector<int> const& vec) {
    return all_of(vec.begin(), vec.end(), is_odd);
  };

  EXPECT_EQ((some(vector<int>{1, 3, 5, 7, 2, 4, 6, 8}).filter(all_odd)), none);
  EXPECT_EQ(
      (some(vector<int>{1, 3, 5, 7}).filter(all_odd).unwrap()),
      (vector<int>{1, 3, 5, 7}));

  EXPECT_EQ(option<vector<int>>(none).filter(all_odd), none);
  EXPECT_EQ(option<vector<int>>(none).filter(all_odd), none);
}

// TEST(OptionTest, FilterNot) {
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

// TEST(OptionTest, Or) {
//  auto&& a = some( 90}).OR(some( 89}));
//  EXPECT_EQ(move(a).unwrap(), 90);

//  auto&& b = make_none<int>().OR(some( 89}));
//  EXPECT_EQ(move(b).unwrap(), 89);

//  auto&& c = make_none<int>().OR(make_none<int>());
//  EXPECT_EQ(c, none);
//  //
//  //
//  auto&& d = some( vector<int>{1, 2, 3, 4, 5}})
//                 .OR(some( vector<int>{6, 7, 8, 9, 10}}));
//  EXPECT_EQ(move(d).unwrap(), (vector<int>{1, 2, 3, 4, 5}));

//  auto&& e =
//      option<vector<int>>(none).OR(some( vector<int>{6, 7, 8, 9,
//      10}}));
//  EXPECT_EQ(move(e).unwrap(), (vector<int>{6, 7, 8, 9, 10}));

//  auto&& f = option<vector<int>>(none).OR(option<vector<int>>(none));
//  EXPECT_EQ(f, none);
//}

// TEST(OptionTest, Xor) {
//  auto&& a = some( 90}).XOR(some( 89}));
//  EXPECT_EQ(a, none);

//  auto&& b = make_none<int>().XOR(some( 89}));
//  EXPECT_EQ(move(b).unwrap(), 89);

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
//  EXPECT_EQ(move(e).unwrap(), (vector<int>{6, 7, 8, 9, 10}));

//  auto&& f = option<vector<int>>(none).XOR(option<vector<int>>(none));
//  EXPECT_EQ(f, none);
//}

TEST(OptionTest, Take) {
  auto a = some(9);
  EXPECT_EQ(a.take().unwrap(), 9);
  EXPECT_EQ(a, none);

  auto b = option<int>(none);
  EXPECT_EQ(b.take(), none);
  EXPECT_EQ(b, none);

  auto c = some(vector<int>{-1, -2, -4, -8, -16});
  auto ca = c.take();
  EXPECT_EQ(ca, (some(vector<int>{-1, -2, -4, -8, -16})));
  EXPECT_EQ(c, none);

  auto d = option<vector<int>>(none);
  EXPECT_EQ(d.take(), none);
  EXPECT_EQ(d, none);
}

// TEST(OptionTest, Replace) {
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

TEST(OptionTest, Clone) {
  auto a = some(9);
  EXPECT_EQ(a.clone(), (some(9)));
  EXPECT_EQ(a, (some(9)));

  auto b = some(static_cast<int*>(nullptr));
  EXPECT_EQ(b.clone(), (some(static_cast<int*>(nullptr))));
  EXPECT_EQ(b, (some(static_cast<int*>(nullptr))));

  auto c = some(vector<int>{1, 2, 3, 4, 5});
  EXPECT_EQ(c.clone(), (some(vector<int>{1, 2, 3, 4, 5})));
  EXPECT_EQ(c, (some(vector<int>{1, 2, 3, 4, 5})));
}

TEST(OptionTest, OrElse) {
  auto&& a = some(90.0F).or_else([]() { return some(0.5f); });
  EXPECT_FLOAT_EQ(move(a).unwrap(), 90.0F);

  auto&& b = option<float>(none).or_else([]() { return some(0.5f); });
  EXPECT_FLOAT_EQ(move(b).unwrap(), 0.5F);

  auto&& c = option<float>(none).or_else([]() { return option<float>(none); });
  EXPECT_EQ(c, none);
  //
  //
  auto&& d = some(vector<int>{1, 2, 3, 4, 5}).or_else([]() {
    return some(vector<int>{6, 7, 8, 9, 10});
  });
  EXPECT_EQ(move(d).unwrap(), (vector<int>{1, 2, 3, 4, 5}));

  auto&& e = option<vector<int>>(none).or_else([]() {
    return some(vector<int>{6, 7, 8, 9, 10});
  });
  EXPECT_EQ(move(e).unwrap(), (vector<int>{6, 7, 8, 9, 10}));

  auto&& f = option<vector<int>>(none).or_else(
      []() { return option<vector<int>>(none); });
  EXPECT_EQ(f, none);
}
