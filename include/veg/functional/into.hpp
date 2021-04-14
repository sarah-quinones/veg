#ifndef VEG_INTO_HPP_PFAECJCZS
#define VEG_INTO_HPP_PFAECJCZS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/tags.hpp"
#include "veg/functional/utils.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace fn {
template <typename T>
struct Into {
	T value;
	VEG_TEMPLATE_CVT(
			typename U,
			requires(VEG_CONCEPT(constructible<U, veg::tags::Into, T&&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
			U) && noexcept(VEG_CONCEPT(nothrow_constructible<U, veg::tags::Into, T&&>)) {
		return U{veg::tags::Into{}, VEG_FWD(value)};
	}
};

namespace nb {
struct into_ref {
	template <typename T>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
	operator()(T&& arg) const noexcept -> Into<T&&> {
		return {VEG_FWD(arg)};
	}
};
} // namespace nb
VEG_NIEBLOID(into_ref);
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INTO_HPP_PFAECJCZS */
