#include "veg/option.hpp"
#include "veg/fn_ref.hpp"
#include <gtest/gtest.h>
#include <vector>

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
    constexpr auto operator()() const -> option<int> { return {some, 13}; }
    constexpr auto operator()(int i) const -> option<double> {
      if (i == 0) {
        return none;
      }
      return {some, 1000. / i};
    }
  };

  struct B {
    constexpr auto operator()(int i) const -> double {
      if (i == 0) {
        return 1.0;
      }
      return 2000. / i;
    }
  };
  struct C {
    constexpr auto operator()() const -> double { return 2000.; }
  };

  static_assert(sizeof(option<int>) == sizeof(int) * 2);
  static_assert(sizeof(option<option<int>>) == sizeof(int) * 2);
  static_assert(sizeof(option<option<option<option<int>>>>) == sizeof(int) * 2);
  static_assert(sizeof(option<fn_ref<void()>>) == sizeof(fn_ref<void()>));
  static_assert(
      sizeof(option<mini_fn_ref<void()>>) == sizeof(mini_fn_ref<void()>));

  constexpr option<int> i = {some, 3};
  constexpr option<int> j = none;
  static_assert(i.as_ref().and_then(A{}).unwrap() == 1000. / 3);
  static_assert(i.as_ref().map(B{}).unwrap() == 2000. / 3);

  static_assert(i.as_ref().map_or_else(B{}, C{}) == 2000. / 3);
  static_assert(j.as_ref().map_or_else(B{}, C{}) == 2000.);
  static_assert(j.as_ref().map_or(B{}, 2000.) == 2000.);
  static_assert(i.as_ref().map_or(B{}, 2000.) == 2000. / 3);
  static_assert(i.as_ref().map([](int k) { return 2.0 * k; }) == some(6.0));

  static_assert(!option<int>{0}.and_then(A{}));
  static_assert(option<int>{3}.and_then(A{}).unwrap() == 1000. / 3);
  static_assert(option<int>{42}.take().unwrap() == 42);
  static_assert(!option<int>{none}.take());
  static_assert(!j.as_ref().and_then(A{}));

  static_assert(option<int>{i}.or_else(A{}).unwrap() == 3);
  static_assert(option<int>{j}.or_else(A{}).unwrap() == 13);

  static_assert(sizeof(option<int&>) == sizeof(int*));

  {
    constexpr option<option<option<int>>> opt = {some, {some, {some, 3}}};
    constexpr option<option<option<int>>> opt_also = some(some(some(3)));
    constexpr option<option<option<int>>> opt2 = {some, {some, {none}}};
    static_assert(opt == opt_also);
    static_assert(
        opt //
            .as_ref()
            .unwrap()
            .as_ref()
            .unwrap()
            .as_ref()
            .unwrap() == 3);
    static_assert(opt.clone().unwrap().unwrap().unwrap() == 3);
    static_assert(!opt2.clone().unwrap().unwrap());
    static_assert(!opt2.clone().flatten().flatten());
    static_assert(opt2.clone().flatten());
    static_assert(opt2.clone().flatten());
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
    constexpr auto opt = [&] {
      option<int> x;
      x.emplace([&] { return 1; });
      return x;
    }();

    static_assert(opt == some(1));
  }
  {
    static_assert(meta::mostly_trivial<int>::value);
    static_assert(meta::mostly_trivial<option<int>>::value);
    constexpr auto opt = [&] {
      option<option<int>> x;
      x.emplace([&] { return some(1); });
      return x;
    }();

    static_assert(opt == some(some(1)));
  }
  {
    using std::vector;

    auto opt = [&] {
      option<vector<int>> x;
      x.emplace([&] { return vector<int>{1, 2, 3}; });
      return x;
    }();

    ASSERT_EQ(opt, some(vector<int>{1, 2, 3}));
  }
  {
    using std::vector;

    static_assert(meta::value_sentinel_for<option<vector<int>>>::value == 253);

    auto opt = [&] {
      option<option<vector<int>>> x;
      x.emplace([&] { return some(vector<int>{1, 2, 3}); });
      return x;
    }();

    ASSERT_EQ(opt, some(some(vector<int>{1, 2, 3})));
  }
  {
    using std::vector;

    static_assert(
        meta::value_sentinel_for<option<option<vector<int>>>>::value == 252);

    auto opt = [&] {
      option<option<option<vector<int>>>> x;
      x.emplace([&] { return some(some(vector<int>{1, 2, 3})); });
      return x;
    }();

    ASSERT_EQ(opt, some(some(some(vector<int>{1, 2, 3}))));
  }
  {
    using std::vector;

    static_assert(
        meta::value_sentinel_for<option<option<option<vector<int>>>>>::value ==
        251);

    auto opt = [&] {
      option<option<option<option<vector<int>>>>> x;
      x.emplace([&] { return some(some(some(vector<int>{1, 2, 3}))); });
      return x;
    }();

    ASSERT_EQ(opt, some(some(some(some(vector<int>{1, 2, 3})))));
  }
}
