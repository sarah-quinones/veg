#ifndef VEG_INTO_HPP_PFAECJCZS
#define VEG_INTO_HPP_PFAECJCZS

#include "veg/type_traits/constructible.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace fn {
template <typename T>
struct Into;

namespace nb {
struct into_ref {
	template <typename T>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
	operator()(T&& arg) const noexcept -> Into<T&&> {
		return {VEG_FWD(arg)};
	}
};
struct into {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(constructible<meta::decay_t<T>, T>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(arg, T&&))
	const noexcept(VEG_CONCEPT(nothrow_constructible<meta::decay_t<T>, T>))
			->Into<meta::decay_t<T>> {
		return {VEG_FWD(arg)};
	}
};
} // namespace nb
VEG_NIEBLOID(into_ref);
VEG_NIEBLOID(into);
} // namespace fn

inline namespace tags {
struct Into : fn::nb::into {};
} // namespace tags

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
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INTO_HPP_PFAECJCZS */
