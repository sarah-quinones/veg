#ifndef VEG_NARROW_HPP_H0EXKJTAS
#define VEG_NARROW_HPP_H0EXKJTAS

#include "veg/util/assert.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace nb {
template <typename To>
struct narrow {
	VEG_TEMPLATE(
			(typename From),
			requires VEG_CONCEPT(integral<From>) && VEG_CONCEPT(integral<To>),
			constexpr,
			auto
			operator(),
			(from, From))
	const noexcept->To {
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
};
} // namespace nb
VEG_IGNORE_CPP14_EXTENSION_WARNING(namespace /* NOLINT */ {
	template <typename To>
	constexpr auto const& narrow =
			internal::meta_::static_const<nb::narrow<To>>::value;
} // namespace
)
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_NARROW_HPP_H0EXKJTAS */
