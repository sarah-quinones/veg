#ifndef VEG_INTCMP_HPP_CVUKT5MIS
#define VEG_INTCMP_HPP_CVUKT5MIS

#include "veg/misc/macros.hpp"
#include "veg/misc/type_traits.hpp"

namespace veg {

namespace _cmp {

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
    return (a >= 0) &&
           (static_cast<typename std::make_unsigned<A>::type>(a) == b);
  }
  template <typename A, typename B>
  static constexpr auto lt(A const& a, B const& b) noexcept -> bool {
    return (a < 0) ||
           (static_cast<typename std::make_unsigned<A>::type>(a) < b);
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
    return (b >= 0) &&
           (static_cast<typename std::make_unsigned<B>::type>(b) < a);
  }
};

template <typename A, typename B>
using eq_t = decltype(VEG_DECLVAL(A const&) == VEG_DECLVAL(B const&));
template <typename A, typename B>
using lt_t = decltype(VEG_DECLVAL(A const&) < VEG_DECLVAL(B const&));

} // namespace _cmp

namespace fn {
struct cmp_equal_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::eq_t, A, B>>::value),
      bool) {
    return _cmp::cmp_impl <
                   (std::is_integral<A>::value && std::is_integral<B>::value &&
                    (std::is_signed<A>::value != std::is_signed<B>::value))
               ? (std::is_signed<A>::value ? _cmp::which::int_signed_unsigned
                                           : _cmp::which::int_unsigned_signed)
               : _cmp::which::generic > ::eq(a, b);
  }
};

struct cmp_not_equal_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::eq_t, A, B>>::value),
      bool) {
    return !cmp_equal_fn{}(a, b);
  }
};

struct cmp_less_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::lt_t, A, B>>::value),
      bool) {
    return _cmp::cmp_impl <
                   (std::is_integral<A>::value && std::is_integral<B>::value &&
                    (std::is_signed<A>::value != std::is_signed<B>::value))
               ? (std::is_signed<A>::value ? _cmp::which::int_signed_unsigned
                                           : _cmp::which::int_unsigned_signed)
               : _cmp::which::generic > ::lt(a, b);
  }
};

struct cmp_greater_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::lt_t, B, A>>::value),
      bool) {
    return cmp_less_fn{}(b, a);
  }
};

struct cmp_less_equal_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::lt_t, B, A>>::value),
      bool) {
    return !cmp_less_fn{}(b, a);
  }
};

struct cmp_greater_equal_fn {
  template <typename A, typename B>
  constexpr auto
  operator()(A const& a, B const& b) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible<bool, tt::detected_t<_cmp::lt_t, A, B>>::value),
      bool) {
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

#endif /* end of include guard VEG_INTCMP_HPP_CVUKT5MIS */
