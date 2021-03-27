#ifndef VEG_VALUE_CATEGORY_HPP_KGISM6ZDS
#define VEG_VALUE_CATEGORY_HPP_KGISM6ZDS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace meta {
enum struct category_e {
	own,
	ref,
	ref_mut,
	ref_mov,
};
} // namespace meta
namespace internal {
namespace meta_ {
template <meta::category_e C>
struct apply_categ;
template <>
struct apply_categ<meta::category_e::own> {
	template <typename T>
	using type = T;
};
template <>
struct apply_categ<meta::category_e::ref> {
	template <typename T>
	using type = T const&;
};
template <>
struct apply_categ<meta::category_e::ref_mut> {
	template <typename T>
	using type = T&;
};
template <>
struct apply_categ<meta::category_e::ref_mov> {
	template <typename T>
	using type = T&&;
};
} // namespace meta_
} // namespace internal
namespace meta {
template <category_e C, typename T>
using apply_category_t =
		typename internal::meta_::apply_categ<C>::template type<T>;

template <typename T>
struct value_category : constant<category_e, category_e::own> {};

template <typename T>
struct value_category<T const&>
		: constant<category_e, category_e::ref> {};
template <typename T>
struct value_category<T const&&>
		: constant<category_e, category_e::ref> {};

template <typename T>
struct value_category<T&>
		: constant<category_e, category_e::ref_mut> {};
template <typename T>
struct value_category<T&&>
		: constant<category_e, category_e::ref_mov> {};

template <typename To, typename From>
using collapse_category_t = typename internal::meta_::apply_categ<
		value_category<From>::value>::template type<To>;
} // namespace meta
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_VALUE_CATEGORY_HPP_KGISM6ZDS */
