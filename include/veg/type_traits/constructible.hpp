#ifndef VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS
#define VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

#if !VEG_HAS_BUILTIN(__has_trivial_destructor) ||                              \
		!VEG_HAS_BUILTIN(__is_trivially_constructible) ||                          \
		!VEG_HAS_BUILTIN(__is_constructible) ||                                    \
		!VEG_HAS_BUILTIN(__is_nothrow_constructible) ||                            \
		!VEG_HAS_BUILTIN(__is_trivially_copyable) ||                               \
		!VEG_HAS_BUILTIN(__is_trivial) || !VEG_HAS_BUILTIN(__is_final) ||          \
		!VEG_HAS_BUILTIN(__is_empty)
#include <type_traits>
#endif

namespace veg {
namespace concepts {

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(typename T, final, T);
VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(typename T, empty, T);

VEG_DEF_CONCEPT(
		typename T, nothrow_destructible, noexcept(static_cast<T*>(nullptr)->~T()));

VEG_DEF_CONCEPT(
		typename T,
		trivially_destructible,
		VEG_HAS_BUILTIN_OR(
				__has_trivial_destructor,
				(__has_trivial_destructor(T)),
				(std::is_trivially_destructible<T>::value)));

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(typename T, trivially_copyable, T);

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(
		typename T, trivially_default_constructible, is_trivially_constructible, T);

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(
		typename T,
		trivially_copy_constructible,
		is_trivially_constructible,
		T,
		T const&);
VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(
		typename T,
		trivially_move_constructible,
		is_trivially_constructible,
		T,
		T&&);

namespace _internal {
template <typename T, typename... Ts>
using inplace_ctor_expr =
		decltype(new (static_cast<void*>(nullptr)) T(VEG_DECLVAL(Ts &&)...));
} // namespace _internal
VEG_DEF_CONCEPT(
		(typename T, typename... Ts),
		inplace_constructible,
		VEG_CONCEPT(detected<_internal::inplace_ctor_expr, T, Ts&&...>));
VEG_DEF_CONCEPT(
		(typename T, typename... Ts),
		nothrow_inplace_constructible,
		VEG_IS_NOEXCEPT(new (static_cast<void*>(nullptr)) T(VEG_DECLVAL(Ts&&)...)));

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename T, typename... Ts), constructible, T, Ts&&...);

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename T, typename... Ts), nothrow_constructible, T, Ts&&...);

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename From, typename To), convertible, From&&, To);

VEG_DEF_CONCEPT(
		(typename T, typename U),
		implicitly_constructible,
		VEG_CONCEPT(convertible<U&&, T>));

VEG_DEF_CONCEPT(
		typename T, move_constructible, VEG_CONCEPT(constructible<T, T&&>));
VEG_DEF_CONCEPT(
		typename T,
		nothrow_move_constructible,
		VEG_CONCEPT(nothrow_constructible<T, T&&>));

VEG_DEF_CONCEPT(
		typename T, copy_constructible, VEG_CONCEPT(constructible<T, T const&>));
VEG_DEF_CONCEPT(
		typename T,
		nothrow_copy_constructible,
		VEG_CONCEPT(nothrow_constructible<T, T const&>));

} // namespace concepts

namespace cpo {
template <typename T>
struct is_trivially_constructible
		: meta::bool_constant<VEG_CONCEPT(trivially_default_constructible<T>)> {};

template <typename T>
struct is_trivially_relocatable
		: meta::bool_constant<
					VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(trivially_move_constructible<T>)> {};

} // namespace cpo

namespace internal {
template <typename From, typename To>
struct ConvertingFn {
	From&& value;
	VEG_INLINE constexpr auto operator()() const&& VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_constructible<From, To>)) -> To {
		return To(VEG_FWD(value));
	}
};
} // namespace internal
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS */
