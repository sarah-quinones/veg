#ifndef VEG_NARROW_HPP_H0EXKJTAS
#define VEG_NARROW_HPP_H0EXKJTAS

#include "veg/assert.hpp"

namespace veg {
namespace fn {
template <typename To>
struct narrow {
  VEG_TEMPLATE(
      (typename From),
      requires meta::arithmetic<From>::value&& meta::arithmetic<To>::value,
      constexpr auto
      operator(),
      (from, From))
  const noexcept->To;
};
template <typename To>
VEG_TEMPLATE(
    (typename From),
    requires meta::arithmetic<From>::value&& meta::arithmetic<To>::value,
    constexpr auto narrow<To>::operator(),
    (from, From))
const noexcept -> To {
  return VEG_ASSERT(static_cast<From>(static_cast<To>(from)) == from),
         static_cast<To>(from);
}
} // namespace fn
namespace {
template <typename To>
constexpr auto const& narrow =
    ::veg::meta::internal::static_const<fn::narrow<To>>::value;
} // namespace
} // namespace veg


#endif /* end of include guard VEG_NARROW_HPP_H0EXKJTAS */
