#ifndef VEG_NARROW_HPP_H0EXKJTAS
#define VEG_NARROW_HPP_H0EXKJTAS

#include "veg/assert.hpp"

namespace veg {
namespace fn {
template <typename To>
struct narrow {
	VEG_TEMPLATE(
			(typename From),
			requires __VEG_CONCEPT(meta::arithmetic<From>) &&
					__VEG_CONCEPT(meta::arithmetic<To>),
			constexpr auto
			operator(),
			(from, From))
	const noexcept->To;
};
template <typename To>
VEG_TEMPLATE(
		(typename From),
		requires __VEG_CONCEPT(meta::arithmetic<From>) &&
				__VEG_CONCEPT(meta::arithmetic<To>),
		constexpr auto narrow<To>::operator(),
		(from, From))
const noexcept -> To {
#if __cplusplus >= 201402L

	To to = static_cast<To>(from);
	From roundtrip_from = static_cast<From>(static_cast<To>(from));
	VEG_ASSERT(roundtrip_from == from);
	return to;

#else
	return VEG_ASSERT(static_cast<From>(static_cast<To>(from)) == from),
				 static_cast<To>(from);
#endif
}
} // namespace fn
__VEG_IGNORE_CPP14_EXTENSION_WARNING(namespace /* NOLINT */ {
	template <typename To>
	constexpr auto const& narrow =
			::veg::meta::internal::static_const<fn::narrow<To>>::value;
} // namespace
)
} // namespace veg

#endif /* end of include guard VEG_NARROW_HPP_H0EXKJTAS */
