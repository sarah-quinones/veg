#include <veg/tuple.hpp>
#include <utility>
#include "doctest.h"

TEST_CASE("tuple") {
  using namespace veg::literals;
  using veg::get;

  veg::tuple<int, char, bool> tup{1, 'c', true};
  veg::tuple<int, char, bool> const tup_c{1, 'c', true};
  veg::tuple<int, char, bool&> const tup_ref{1, 'c', get<2>(tup)};

  CHECK(get<0>(tup) == 1);
  CHECK(get<1>(tup) == 'c');
  CHECK(get<2>(tup));
  CHECK(&get<2>(tup) == &get<2>(tup_ref));

#if __cplusplus >= 201703L
  auto [i, c, b] = [&] { return tup; }();
  CHECK(i == 1);
  CHECK(c == 'c');
  CHECK(b);
  veg::tuple tup_deduce{1, 'c', true};
#endif

#define ASSERT_SAME(...)                                                       \
  static_assert(::std::is_same<__VA_ARGS__>::value, "fail")

  static_assert(std::is_copy_assignable<veg::tuple<int, char>>(), "fail");
  static_assert(std::is_trivially_copyable<veg::tuple<int, char>>(), "fail");
  static_assert(!std::is_copy_assignable<veg::tuple<int&, char&>>(), "fail");
  static_assert(!std::is_copy_assignable<veg::tuple<int&>>(), "fail");
  ASSERT_SAME(decltype(get<0>(tup)), int&);
  ASSERT_SAME(decltype(get<0>(tup)), decltype(tup[0_c]));
  ASSERT_SAME(decltype(get<0>(VEG_MOV(tup))), decltype(VEG_MOV(tup)[0_c]));
  ASSERT_SAME(decltype(get<1>(tup_c)), char const&);
  ASSERT_SAME(decltype(get<0>(VEG_MOV(tup))), int&&);
  ASSERT_SAME(decltype(get<1>(VEG_MOV(tup_c))), char const&&);
#if __cplusplus >= 201703L
  ASSERT_SAME(decltype(i), int);
  ASSERT_SAME(decltype((i)), int&);
  ASSERT_SAME(decltype((b)), bool&);
  ASSERT_SAME(decltype(tup_deduce), veg::tuple<int, char, bool>);
#endif
}
