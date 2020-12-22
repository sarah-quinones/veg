#include <veg/fn_ref.hpp>
#include <veg/option.hpp>
#include "doctest.h"

TEST_CASE("no args") {
  static int global = 0;
  int i = 0;
  auto inc_lambda = [&i] { ++i; };
  auto inc2_lambda = [&i] { i += 2; };
  auto returns_lambda = [&i] {
    i += 3;
    return i;
  };

  void (*inc_fn_ptr)() = +[] { ++global; };
  auto inc2_global_lambda = [] { global += 2; };
  auto returns_fn_ptr = +[] {
    global += 3;
    return global;
  };

  veg::mini_fn_ref<void()> fn_ref{inc_lambda};
  CHECK(i == 0);
  fn_ref();
  CHECK(i == 1);
  fn_ref();
  CHECK(i == 2);

  fn_ref = inc2_lambda;
  fn_ref();
  CHECK(i == 4);

  fn_ref = returns_lambda;
  fn_ref();
  CHECK(i == 7);

  fn_ref = *inc_fn_ptr;
  CHECK(global == 0);
  fn_ref();
  CHECK(global == 1);

  fn_ref = inc2_global_lambda;
  fn_ref();
  CHECK(global == 3);

  fn_ref = +returns_fn_ptr;
  fn_ref = *+returns_fn_ptr;
  fn_ref();
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

TEST_CASE("member functions") {
  foo a;
  foo b;
  veg::fn_ref<foo(foo&, foo, int)> fn(&foo::bar);

  foo _ = fn(a, {}, 1);
  CHECK(global == &a);
  fn = &foo::bar2;
  _ = fn(b, {}, 1);
  CHECK(global == (&b + 1));

  fn = baz;
  _ = fn(b, {}, 1);
  CHECK(global == nullptr);
}

TEST_CASE("null") {
  veg::option<veg::fn_ref<void()>> f;
  CHECK(!f);
  f = {veg::some, [] {}};
  CHECK(f);
}
