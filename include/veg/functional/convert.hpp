#ifndef VEG_CONVERT_HPP_3AYTBOLMS
#define VEG_CONVERT_HPP_3AYTBOLMS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace fn {

template <typename From, typename To>
struct ConvertFn {
	From value;
	VEG_INLINE constexpr auto operator()() const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, From const&>))
					-> To {
		return To(VEG_FWD(value));
	}
};

namespace nb {
template <typename To>
struct convert {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_CONCEPT(constructible<To, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Ts&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, Ts...>)) -> To {
		return To(VEG_FWD(args)...);
	}
};
} // namespace nb

} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CONVERT_HPP_3AYTBOLMS */
