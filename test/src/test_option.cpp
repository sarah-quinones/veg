#include "veg/option.hpp"
#include "veg/fn_ref.hpp"
#include <gtest/gtest.h>

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

  constexpr option<int> i = {some, 3};
  constexpr option<int> j = none;
  static_assert(i.and_then(A{}).unwrap() == 1000. / 3);
  static_assert(i.map(B{}).unwrap() == 2000. / 3);

  static_assert(i.map_or_else(B{}, C{}) == 2000. / 3);
  static_assert(j.map_or_else(B{}, C{}) == 2000.);
  static_assert(j.map_or(B{}, 2000) == 2000.);
  static_assert(i.map_or(B{}, 2000) == 2000. / 3);

  static_assert(!option<int>{0}.and_then(A{}));
  static_assert(option<int>{3}.and_then(A{}).unwrap() == 1000. / 3);
  static_assert(option<int>{42}.take().unwrap() == 42);
  static_assert(!option<int>{none}.take());
  static_assert(!j.and_then(A{}));

  static_assert(option<int>{i}.or_else(A{}).unwrap() == 3);
  static_assert(option<int>{j}.or_else(A{}).unwrap() == 13);

  static_assert(sizeof(option<int&>) == sizeof(int*));

  static_assert(sizeof(option<int>) == sizeof(int) * 2);
  static_assert(sizeof(option<option<int>>) == sizeof(int) * 2);
  static_assert(sizeof(option<option<option<option<int>>>>) == sizeof(int) * 2);
  static_assert(sizeof(option<fn_ref<void()>>) == sizeof(fn_ref<void()>));
  static_assert(
      sizeof(option<mini_fn_ref<void()>>) == sizeof(mini_fn_ref<void()>));

  {
    constexpr option<option<option<int>>> opt = {some, {some, {some, 3}}};
    constexpr option<option<option<int>>> opt2 = {some, {some, {none}}};
    static_assert(opt.unwrap().unwrap().unwrap() == 3);
    static_assert(!opt2.unwrap().unwrap());
    static_assert(!opt2.flatten().flatten());
    static_assert(opt2.flatten());
    static_assert(opt2.flatten());
  }

  {
    option<bool> flag;
    option<int> opt;
    opt.map_or_else(
        [&](int /*unused*/) {
          flag = {some, true};
        },
        [&] {
          flag = {some, false};
        });
    ASSERT_TRUE(flag);
    ASSERT_FALSE(flag.unwrap());

    opt = {some, 3};
    flag = {};
    opt.map_or_else(
        [&](int /*unused*/) {
          flag = {some, true};
        },
        [&] {
          flag = {some, false};
        });
    ASSERT_TRUE(flag);
    ASSERT_TRUE(flag.unwrap());
  }
}
