#ifndef __VEG_CMP_HPP_6QBW4XNOS
#define __VEG_CMP_HPP_6QBW4XNOS

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace internal {
namespace cmp {
enum struct which {
  int_signed_unsigned,
  int_unsigned_signed,
  floating_point,
  generic,
};

template <which Signedness /* same_sign */>
struct cmp_impl {
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
      -> bool {
    return static_cast<bool>(a == b);
  }
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
      -> bool {
    return static_cast<bool>(a < b);
  }
};

template <>
struct cmp_impl<which::floating_point> {
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
      -> bool {
    using common = decltype(a + b);
    return static_cast<common>(a) == static_cast<common>(b);
  }
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
      -> bool {
    using common = decltype(a + b);
    return static_cast<common>(a) < static_cast<common>(b);
  }
};

template <>
struct cmp_impl<which::int_signed_unsigned> {
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
      -> bool {
    return (a >= 0) && (static_cast<meta::make_unsigned_t<A>>(a) == b);
  }
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
      -> bool {
    return (a < 0) || (static_cast<meta::make_unsigned_t<A>>(a) < b);
  }
};

template <>
struct cmp_impl<which::int_unsigned_signed> {
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
      -> bool {
    return cmp_impl<which::int_signed_unsigned>::eq(b, a);
  }
  template <typename A, typename B>
  HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
      -> bool {
    return (b >= 0) && (static_cast<meta::make_unsigned_t<B>>(b) < a);
  }
};

template <typename A, typename B>
using cmp_impl_ = internal::cmp::cmp_impl<
    (meta::signed_integral<A>::value && meta::unsigned_integral<B>::value)  //
        ? which::int_signed_unsigned                                        //
        : (meta::unsigned_integral<A>::value&&                              //
               meta::signed_integral<B>::value)                             //
              ? which::int_unsigned_signed                                  //
              : (meta::arithmetic<A>::value&& meta::arithmetic<B>::value && //
                 (meta::floating_point<A>::value ||                         //
                  meta::floating_point<B>::value))                          //
                    ? which::floating_point                                 //
                    : which::generic                                        //
    >;
} // namespace cmp
} // namespace internal

namespace fn {
struct cmp_equal {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::equality_comparable_with<A, B>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return internal::cmp::cmp_impl_<A, B>::eq(a, b); }
};

struct cmp_not_equal {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::equality_comparable_with<A, B>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_equal{}(a, b); }
};

struct cmp_less {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::partially_ordered_with<A, B>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return internal::cmp::cmp_impl_<A, B>::lt(a, b); }
};

struct cmp_greater {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::partially_ordered_with<B, A>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return cmp_less{}(b, a); }
};

struct cmp_less_equal {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::partially_ordered_with<B, A>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_less{}(b, a); }
};

struct cmp_greater_equal {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::partially_ordered_with<A, B>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_less{}(a, b); }
};

} // namespace fn
__VEG_ODR_VAR(cmp_equal, fn::cmp_equal);
__VEG_ODR_VAR(cmp_not_equal, fn::cmp_not_equal);
__VEG_ODR_VAR(cmp_less, fn::cmp_less);
__VEG_ODR_VAR(cmp_greater, fn::cmp_greater);
__VEG_ODR_VAR(cmp_less_equal, fn::cmp_less_equal);
__VEG_ODR_VAR(cmp_greater_equal, fn::cmp_greater_equal);

} // namespace veg

#endif /* end of include guard __VEG_CMP_HPP_6QBW4XNOS */
