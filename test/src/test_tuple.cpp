#include <veg/tuple.hpp>
#include <utility>
#include <gtest/gtest.h>

#define MOV VEG_MOV

TEST(tuple, all) {
  using namespace veg::literals;
  using veg::get;

  veg::tuple<int, char, bool> tup{1, 'c', true};
  veg::tuple<int const, char const, bool> tup_c{1, 'c', true};
  veg::tuple<int, char&&, char, bool&, bool const&> tup_ref{
      1, MOV(tup).as_ref()[1_c], 'c', get<2>(tup), get<2>(tup)};

  {
    static_assert(veg::meta::trivially_relocatable<decltype(tup)>::value);
    static_assert(!std::is_copy_constructible<decltype(tup_ref)>::value);
    static_assert(std::is_copy_constructible<decltype(tup)>::value);
    static_assert(std::is_copy_constructible<veg::tuple<int&, bool&>>::value);
    using val_tup = veg::tuple<int, bool>;
    using ref_tup = veg::tuple<int&, bool&>;
    static_assert(veg::meta::swappable<ref_tup&, ref_tup&>::value);
    static_assert(veg::meta::swappable<val_tup&, val_tup&>::value);
  }
  {
    using val_tup = veg::tuple<int, bool, int&>;
    int i{};
    val_tup a{5, true, i};
    val_tup b{3, false, a[0_c]};
    swap(a, b);
    EXPECT_EQ(a[0_c], 3);
    EXPECT_EQ(b[0_c], 5);
    EXPECT_EQ(a[1_c], false);
    EXPECT_EQ(b[1_c], true);
    EXPECT_EQ(&a[2_c], &a[0_c]);
    EXPECT_EQ(&b[2_c], &i);
  }

  EXPECT_EQ(get<0>(tup), 1);
  EXPECT_EQ(get<1>(tup), 'c');
  EXPECT_TRUE(get<2>(tup));

  {
    auto&& ref = VEG_MOV(tup)[2_c];
    auto&& ref2 = VEG_MOV(tup).as_ref()[2_c];
    EXPECT_NE(&ref, &tup[2_c]);
    EXPECT_EQ(&ref2, &tup[2_c]);
  }

  VEG_BIND(auto, (e, f, g), [&] { return tup; }());
#if __cplusplus >= 201703L
  auto [i, c, b] = [&] { return tup; }();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(c, 'c');
  EXPECT_TRUE(b);
  veg::tuple tup_deduce{1, 'c', true};
#endif

#define ASSERT_SAME(...)                                                       \
  static_assert(::std::is_same<__VA_ARGS__>::value, "fail")
  static_assert(std::is_copy_assignable<veg::tuple<int, char>>(), "fail");
  static_assert(std::is_trivially_copyable<veg::tuple<int, char>>(), "fail");
  static_assert(std::is_copy_assignable<veg::tuple<int&, char&>>(), "fail");
  static_assert(std::is_copy_assignable<veg::tuple<int&>>(), "fail");
  ASSERT_SAME(decltype(get<0>(tup)), int&);
  ASSERT_SAME(decltype(get<0>(tup)), decltype(tup[0_c]));
  ASSERT_SAME(decltype(get<0>(MOV(tup))), decltype(MOV(tup)[0_c]));
  ASSERT_SAME(decltype(get<1>(tup_c)), char const&);
  ASSERT_SAME(decltype(get<0>(MOV(tup))), int);
  ASSERT_SAME(decltype(get<0>(MOV(tup).as_ref())), int&&);
  ASSERT_SAME(decltype(get<1>(MOV(tup_c))), char);

  ASSERT_SAME(decltype(tup.as_ref()), veg::tuple<int&, char&, bool&>);
  ASSERT_SAME(
      decltype(tup_c.as_ref()), veg::tuple<int const&, char const&, bool&>);
  ASSERT_SAME(decltype(MOV(tup).as_ref()), veg::tuple<int&&, char&&, bool&&>);

  ASSERT_SAME(decltype(e), int&&);
  ASSERT_SAME(decltype((e)), int&);
  ASSERT_SAME(decltype(f), char&&);
  ASSERT_SAME(decltype((f)), char&);
  ASSERT_SAME(decltype((g)), bool&);

  ASSERT_SAME(decltype(tup_ref[0_c]), int&);
  ASSERT_SAME(decltype(MOV(tup_ref).as_ref()[0_c]), int&&);

  ASSERT_SAME(decltype(tup_ref[1_c]), char&);
  ASSERT_SAME(decltype(tup_ref[2_c]), char&);
  ASSERT_SAME(decltype(MOV(tup_ref)[1_c]), char&&);
  ASSERT_SAME(decltype(MOV(tup_ref)[2_c]), char);
  ASSERT_SAME(decltype(MOV(tup_ref).as_ref()[2_c]), char&&);

  ASSERT_SAME(decltype(tup_ref[3_c]), bool&);
  ASSERT_SAME(decltype(tup_ref[4_c]), bool const&);
  ASSERT_SAME(decltype(MOV(tup_ref)[3_c]), bool&);
  ASSERT_SAME(decltype(MOV(tup_ref)[4_c]), bool const&);

#if __cplusplus >= 201703L
  ASSERT_SAME(decltype(i), int);
  ASSERT_SAME(decltype((i)), int&);
  ASSERT_SAME(decltype((b)), bool&);
  ASSERT_SAME(decltype(tup_deduce), veg::tuple<int, char, bool>);
#endif
}

TEST(tuple, nested) {
  using namespace veg;
  tuple<int, tuple<int, float>> tup{1, {2, 3.0F}};
  ASSERT_EQ(tup[0_c], 1);

  ASSERT_EQ(tup[1_c][1_c], 3.0F);
  ASSERT_EQ(tup[1_c][0_c], 2);
  ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
  ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
  ASSERT_SAME(decltype(VEG_MOV(tup)[1_c][0_c]), int);
}
