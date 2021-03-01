#ifndef VEG_BYTE_STRING_REF_HPP_NK1AVIX7S
#define VEG_BYTE_STRING_REF_HPP_NK1AVIX7S

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace assert {
namespace internal {
struct char_string_ref {
  HEDLEY_ALWAYS_INLINE constexpr char_string_ref(
      char const* data, i64 len) noexcept
      : data_{data}, len_{len} {}

  char_string_ref // NOLINT(hicpp-explicit-conversions)
      (char const* str) noexcept;

  VEG_TEMPLATE(
      typename T,
      requires(
          (meta::constructible<
               char const*,
               decltype(__VEG_DECLVAL(T const&).data())>::value && //
           meta::constructible<i64, decltype(__VEG_DECLVAL(T const&).size())>::
               value)),

      HEDLEY_ALWAYS_INLINE constexpr char_string_ref, // NOLINT(hicpp-explicit-conversions)
      (arg, T const&))
  noexcept
      : char_string_ref{
            static_cast<char const*>(arg.data()),
            static_cast<i64>(arg.size())} {}

  char const* data_;
  i64 len_;

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto data() const noexcept
      -> char const* {
    return data_;
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto size() const noexcept
      -> i64 {
    return len_;
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto
  starts_with(char_string_ref other) const noexcept -> bool;
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto
  operator==(char_string_ref other) const noexcept -> bool;
};
template <typename T>
struct byte_str_static_const {
  static constexpr T value = {"", 0};
};
template <typename T>
constexpr T byte_str_static_const<
    T>::value; // NOLINT(readability-redundant-declaration)
namespace /* NOLINT */ {
constexpr auto const& empty_str /* NOLINT */ =
    byte_str_static_const<char_string_ref>::value;
} // namespace

} // namespace internal
} // namespace assert
} // namespace veg

#endif /* end of include guard VEG_BYTE_STRING_REF_HPP_NK1AVIX7S */
