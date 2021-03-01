#include "static_assert.hpp"
#include <veg/fn_ref.hpp>
#include <veg/option.hpp>
#include <gtest/gtest.h>

using namespace veg;
TEST(function_ref, no_args) {
  static int global = 0;
  int i = 0;
  auto inc_lambda = [&i] { ++i; };
  auto inc2_lambda = [&i] { i += 2; };
  auto returns_lambda = [&i] {
    i += 3;
    return i;
  };

  void (*p)(){};
  using T = decltype(p);

  STATIC_ASSERT(noexcept(p == nullptr));
  STATIC_ASSERT(
      meta::internal::equality_comparable_impl<T, std::nullptr_t>::value);

  void (*inc_fn_ptr)() = +[] { ++global; };
  auto inc2_global_lambda = [] { global += 2; };
  auto returns_fn_ptr = +[] {
    global += 3;
    return global;
  };

  mini_fn_ref<void()> fn_ref{inc_lambda};
  EXPECT_EQ(i, 0);
  fn_ref();
  EXPECT_EQ(i, 1);
  fn_ref();
  EXPECT_EQ(i, 2);

  fn_ref = inc2_lambda;
  fn_ref();
  EXPECT_EQ(i, 4);

  fn_ref = returns_lambda;
  fn_ref();
  EXPECT_EQ(i, 7);

  fn_ref = *inc_fn_ptr;
  EXPECT_EQ(global, 0);
  fn_ref();
  EXPECT_EQ(global, 1);

  fn_ref = inc2_global_lambda;
  fn_ref();
  EXPECT_EQ(global, 3);

  fn_ref = +returns_fn_ptr;
  fn_ref = *+returns_fn_ptr;
  fn_ref();
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

TEST(function_ref, member_functions) {
  foo a;
  foo b;
  fn_ref<foo(foo&, foo, int)> fn(&foo::bar);

  foo _ = fn(a, {}, 1);
  EXPECT_EQ(global, &a);
  fn = &foo::bar2;
  _ = fn(b, {}, 1);
  EXPECT_EQ(global, (&b + 1));

  fn = baz;
  _ = fn(b, {}, 1);
  EXPECT_EQ(global, nullptr);
}

TEST(function_ref, null) {
  option<fn_ref<void()>> f;

  EXPECT_TRUE(!f);
  EXPECT_DEATH({ void(f.as_ref().unwrap()); }, "");
  EXPECT_DEATH({ void(VEG_MOV(f).unwrap()); }, "");

  f = {some, [] {}};
  EXPECT_TRUE(f);

  void (*null)() = nullptr;

  auto make_null = [&] { f.as_ref().unwrap() = {null}; };
  EXPECT_DEATH({ make_null(); }, "");
}
