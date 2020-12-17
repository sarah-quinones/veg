#include "veg/dynamic_stack.hpp"
#include "doctest.h"

#include <atomic>

struct S {
private:
  static auto n_instances_mut() -> std::atomic<int>& {
    static std::atomic<int> var{0};
    return var;
  }

public:
  static auto n_instances() -> int { return n_instances_mut(); }

  S(S const&) = delete;
  S(S&&) = delete;
  auto operator=(S const&) -> S& = delete;
  auto operator=(S&&) -> S& = delete;
  S() { ++n_instances_mut(); }
  ~S() { --n_instances_mut(); }
};

TEST_CASE("dynamic-stack-raii") {
  unsigned char buf[4096];

  veg::dynamic_stack_view stack{veg::make::slice(buf)};
  using veg::tag;

  {
    auto s1 = stack.make_new(tag<S>, 3);
    CHECK(s1.data() != nullptr);
    CHECK(s1.size() == 3);
    CHECK(stack.remaining_bytes() == 4093);
    CHECK(S::n_instances() == 3);

    {
      auto s2 = stack.make_new(tag<S>, 4);
      CHECK(s2.data() != nullptr);
      CHECK(s2.size() == 4);
      CHECK(stack.remaining_bytes() == 4089);
      CHECK(S::n_instances() == 7);

      {
        auto i3 = stack.make_new(tag<int>, 30000);
        CHECK(i3.data() == nullptr);
        CHECK(i3.size() == 0);
        CHECK(stack.remaining_bytes() == 4089);
        {
          auto i4 = stack.make_new(tag<int>, 300);
          CHECK(i4.data() != nullptr);
          CHECK(i4.size() == 300);
          CHECK(stack.remaining_bytes() < 4089 - 300 * sizeof(int));
        }
      }
    }
    CHECK(stack.remaining_bytes() == 4093);
    CHECK(S::n_instances() == 3);
  }
  CHECK(stack.remaining_bytes() == 4096);
  CHECK(S::n_instances() == 0);

  auto s1 = stack.make_new(tag<S const>, 3);
  CHECK(stack.remaining_bytes() == 4093);
  CHECK(S::n_instances() == 3);
}

TEST_CASE("dynamic-stack-return") {
  unsigned char buf[4096];
  veg::dynamic_stack_view stack(veg::make::slice(buf));
  using veg::tag;

  auto s = [&] {
    auto s1 = stack.make_new(tag<S>, 3);
    auto s2 = stack.make_new(tag<S>, 4);
    auto s3 = stack.make_new(tag<S>, 5);
    CHECK(stack.remaining_bytes() == 4084);
    CHECK(S::n_instances() == 12);
    return s1;
  }();

  CHECK(stack.remaining_bytes() == 4093);
  CHECK(S::n_instances() == 3);

  CHECK(s.data() != nullptr);
  CHECK(s.size() == 3);
}

TEST_CASE("dynamic-stack-manual-lifetimes") {
  unsigned char buf[4096];
  veg::dynamic_stack_view stack(veg::make::slice(buf));
  using veg::tag;

  auto s = stack.make_alloc(tag<S>, 3);
  CHECK(s.data() != nullptr);
  CHECK(s.size() == 3);
  CHECK(S::n_instances() == 0);

  {
    new (s.data() + 0) S{};
    CHECK(S::n_instances() == 1);
    new (s.data() + 1) S{};
    CHECK(S::n_instances() == 2);
    new (s.data() + 2) S{};
    CHECK(S::n_instances() == 3);

    (s.data() + 2)->~S();
    CHECK(S::n_instances() == 2);
    (s.data() + 1)->~S();
    CHECK(S::n_instances() == 1);
    (s.data() + 0)->~S();
    CHECK(S::n_instances() == 0);
  }
  CHECK(S::n_instances() == 0);
}
