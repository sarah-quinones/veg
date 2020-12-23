#include <veg/internal/meta_int.hpp>
#include <ostream>
#include <gtest/gtest.h>

TEST(meta_int, all) {
  using namespace veg;
  using namespace veg::literals;

  static_assert(std::is_constructible<fix<3>, fix<3>>::value, "fail");
  static_assert(std::is_constructible<fix<3>, dyn>::value, "fail");
  static_assert(std::is_constructible<dyn, dyn>::value, "fail");
  static_assert(std::is_constructible<dyn, fix<3>>::value, "fail");
  static_assert(std::is_constructible<dyn, i64>::value, "fail");
  static_assert(std::is_constructible<fix<3>, i64>::value, "fail");

  static_assert(!std::is_constructible<fix<3>, fix<4>>::value, "fail");

  EXPECT_NE(0_c, 2_c);
  EXPECT_EQ(0_c + 2_c, 2_c);
  EXPECT_EQ(0_c + dyn(2_c), 2_c);
  EXPECT_EQ(dyn(0_c) + dyn(2_c), dyn(2_c));
  EXPECT_EQ(0_v + 2_v, 2_v);
  EXPECT_EQ(2_c - 2_c, 0_c);
  EXPECT_EQ(2_c - 1_c - 1_c - 1_c, -dyn(1_c));

  EXPECT_TRUE(boolean<maybe>(true));
  EXPECT_TRUE(!boolean<maybe>(false));
  EXPECT_TRUE(!boolean<maybe>());
  EXPECT_TRUE(!boolean<no>());
  EXPECT_TRUE(boolean<yes>());
  EXPECT_DEATH({ fix<2>{3}; }, "");
  EXPECT_DEATH({ void(2_c / 0_v); }, "");
}
