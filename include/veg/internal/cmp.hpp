#ifndef VEG_CMP_HPP_6QBW4XNOS
#define VEG_CMP_HPP_6QBW4XNOS

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace internal {
namespace cmp {
enum struct which {
  int_signed_unsigned,
  int_unsigned_signed,
  generic,
};

template <which Signedness /* same_sign */>
struct cmp_impl {
  template <typename A, typename B>
  static constexpr auto eq(A const& a, B const& b) noexcept(noexcept(a == b)) {
    return a == b;
  }
  template <typename A, typename B>
  static constexpr auto lt(A const& a, B const& b) noexcept(noexcept(a < b)) {
    return a < b;
  }
};

template <>
struct cmp_impl<which::int_signed_unsigned> {
  template <typename A, typename B>
  static constexpr auto eq(A const& a, B const& b) noexcept -> bool {
    return (a >= 0) && (static_cast<meta::make_unsigned_t<A>>(a) == b);
  }
  template <typename A, typename B>
  static constexpr auto lt(A const& a, B const& b) noexcept -> bool {
    return (a < 0) || (static_cast<meta::make_unsigned_t<A>>(a) < b);
  }
};

template <>
struct cmp_impl<which::int_unsigned_signed> {
  template <typename A, typename B>
  static constexpr auto eq(A const& a, B const& b) noexcept -> bool {
    return cmp_impl<which::int_signed_unsigned>::eq(b, a);
  }
  template <typename A, typename B>
  static constexpr auto lt(A const& a, B const& b) noexcept -> bool {
    return (b >= 0) && (static_cast<meta::make_unsigned_t<B>>(b) < a);
  }
};
} // namespace cmp
} // namespace internal

namespace fn {
struct cmp_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_equality_comparable_with<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(

      internal::cmp::cmp_impl<
          (meta::is_integral<A>::value && meta::is_integral<B>::value &&
           meta::is_signed<A>::value != meta::is_signed<B>::value)
              ? (meta::is_signed<A>::value
                     ? internal::cmp::which::int_signed_unsigned
                     : internal::cmp::which::int_unsigned_signed)
              : internal::cmp::which::generic>::eq(a, b)

          )) {
    return internal::cmp::cmp_impl <
                   (meta::is_integral<A>::value &&
                    meta::is_integral<B>::value &&
                    meta::is_signed<A>::value != meta::is_signed<B>::value)
               ? (meta::is_signed<A>::value
                      ? internal::cmp::which::int_signed_unsigned
                      : internal::cmp::which::int_unsigned_signed)
               : internal::cmp::which::generic > ::eq(a, b);
  }
};

struct cmp_not_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_equality_comparable_with<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(!cmp_equal_fn{}(a, b))) {
    return !cmp_equal_fn{}(a, b);
  }
};

struct cmp_less_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_partially_ordered_with<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(

      internal::cmp::cmp_impl<
          (meta::is_integral<A>::value && meta::is_integral<B>::value &&
           meta::is_signed<A>::value != meta::is_signed<B>::value)
              ? (meta::is_signed<A>::value
                     ? internal::cmp::which::int_signed_unsigned
                     : internal::cmp::which::int_unsigned_signed)
              : internal::cmp::which::generic>::lt(a, b)

          )) {
    return internal::cmp::cmp_impl <
                   (meta::is_integral<A>::value &&
                    meta::is_integral<B>::value &&
                    meta::is_signed<A>::value != meta::is_signed<B>::value)
               ? (meta::is_signed<A>::value
                      ? internal::cmp::which::int_signed_unsigned
                      : internal::cmp::which::int_unsigned_signed)
               : internal::cmp::which::generic > ::lt(a, b);
  }
};

struct cmp_greater_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_partially_ordered_with<B, A>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(cmp_less_fn{}(b, a))) { return cmp_less_fn{}(b, a); }
};

struct cmp_less_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_partially_ordered_with<B, A>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(!cmp_less_fn{}(b, a))) {
    return !cmp_less_fn{}(b, a);
  }
};

struct cmp_greater_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(meta::is_partially_ordered_with<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept(noexcept(!cmp_less_fn{}(a, b))) {
    return !cmp_less_fn{}(a, b);
  }
};

} // namespace fn
VEG_ODR_VAR(cmp_equal, fn::cmp_equal_fn);
VEG_ODR_VAR(cmp_not_equal, fn::cmp_not_equal_fn);
VEG_ODR_VAR(cmp_less, fn::cmp_less_fn);
VEG_ODR_VAR(cmp_greater, fn::cmp_greater_fn);
VEG_ODR_VAR(cmp_less_equal, fn::cmp_less_equal_fn);
VEG_ODR_VAR(cmp_greater_equal, fn::cmp_greater_equal_fn);

} // namespace veg

#endif /* end of include guard VEG_CMP_HPP_6QBW4XNOS */
