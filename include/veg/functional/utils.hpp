#ifndef VEG_HELPERS_HPP_6TYDENKMS
#define VEG_HELPERS_HPP_6TYDENKMS

#include "veg/type_traits/constructible.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace nb {
struct clone {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(constructible<meta::decay_t<T>, T>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(arg, T&&))
	const noexcept->meta::decay_t<T> { return meta::decay_t<T>(VEG_FWD(arg)); }
};
struct ref {
	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T&& arg) const noexcept
			-> T&& {
		return VEG_FWD(arg);
	}
};
} // namespace nb
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_HELPERS_HPP_6TYDENKMS */
