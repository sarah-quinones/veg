#ifndef VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS
#define VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

#if !VEG_HAS_BUILTIN(__has_trivial_destructor) ||                              \
		!VEG_HAS_BUILTIN(__is_trivially_constructible) ||                          \
		!VEG_HAS_BUILTIN(__is_constructible) ||                                    \
		!VEG_HAS_BUILTIN(__is_nothrow_constructible) ||                            \
		!VEG_HAS_BUILTIN(__is_trivially_copyable) || !VEG_HAS_BUILTIN(__is_trivial)
#include <type_traits>
#endif

namespace veg {
inline namespace VEG_ABI {
namespace concepts {

VEG_DEF_CONCEPT(
		typename T,
		trivially_destructible,
		VEG_HAS_BUILTIN_OR(
				__has_trivial_destructor,
				(__has_trivial_destructor(T)),
				(std::is_trivially_destructible<T>::value)));

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(typename T, trivially_copyable, T);
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

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename T, typename... Ts), constructible, T, Ts&&...);
VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename T, typename... Ts), nothrow_constructible, T, Ts&&...);

VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(
		(typename From, typename To), convertible, From&&, To);

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

namespace meta {

template <typename T>
struct is_mostly_trivial : VEG_HAS_BUILTIN_OR(
															 __is_trivial,
															 (bool_constant<__is_trivial(T)>),
															 (std::is_trivial<T>)) {};

template <typename T>
struct is_trivially_relocatable
		: bool_constant<
					VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(trivially_move_constructible<T>)> {};

} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(typename T, mostly_trivial, meta::is_mostly_trivial<T>::value);
VEG_DEF_CONCEPT(
		typename T,
		trivially_relocatable,
		meta::is_trivially_relocatable<T>::value);
} // namespace concepts
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CONSTRUCTIBLE_HPP_D6CRVBJYS */
