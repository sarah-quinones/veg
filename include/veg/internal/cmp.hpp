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
  static constexpr auto eq(A const& a, B const& b) noexcept -> bool {
    return static_cast<bool>(a == b);
  }
  template <typename A, typename B>
  static constexpr auto lt(A const& a, B const& b) noexcept -> bool {
    return static_cast<bool>(a < b);
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

template <typename A, typename B, typename Enable = void>
struct is_equality_comparable_impl : std::false_type {};
template <typename A, typename B>
struct is_equality_comparable_impl<
    A,
    B,
    decltype(void(
        static_cast<bool>(VEG_DECLVAL(A const&) == VEG_DECLVAL(B const&))))>

    : meta::disjunction<
          meta::is_same<A, B>,
          meta::negation<meta::disjunction<std::is_enum<A>, std::is_enum<B>>>> {
};

template <typename A, typename B>
struct is_equality_comparable
    : meta::disjunction<
          meta::bool_constant<
              meta::is_scalar<A>::value && meta::is_scalar<B>::value>,
          is_equality_comparable_impl<A, B>> {};

template <typename A, typename B, typename Enable = void>
struct is_less_than_comparable : std::false_type {};
template <typename A, typename B>
struct is_less_than_comparable<
    A,
    B,
    decltype(void(VEG_DECLVAL(A const&) < VEG_DECLVAL(B const&)))> :

    meta::bool_constant<
        (std::is_enum<A>::value && std::is_same<A, B>::value) ||

        (!std::is_enum<A>::value && //
         !std::is_enum<B>::value)> {};
} // namespace cmp
} // namespace internal

namespace fn {
struct cmp_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(internal::cmp::is_equality_comparable<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool {
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
      requires(internal::cmp::is_equality_comparable<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_equal_fn{}(a, b); }
};

struct cmp_less_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(internal::cmp::is_less_than_comparable<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool {
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
      requires(internal::cmp::is_less_than_comparable<B, A>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return cmp_less_fn{}(b, a); }
};

struct cmp_less_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(internal::cmp::is_less_than_comparable<B, A>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_less_fn{}(b, a); }
};

struct cmp_greater_equal_fn {
  VEG_TEMPLATE(
      (typename A, typename B),
      requires(internal::cmp::is_less_than_comparable<A, B>::value),
      constexpr auto
      operator(),
      (a, A const&),
      (b, B const&))
  const noexcept->bool { return !cmp_less_fn{}(a, b); }
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
