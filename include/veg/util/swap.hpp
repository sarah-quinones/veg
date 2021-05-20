#ifndef VEG_SWAP_HPP_CTDUPQCUS
#define VEG_SWAP_HPP_CTDUPQCUS

#include "veg/type_traits/core.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/assignable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace meta_ {

struct adl_fn_swap {
	template <typename U, typename V>
	using type = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));

	template <typename U, typename V>
	VEG_INLINE static VEG_CPP14(constexpr) void apply(U&& u, V&& v) noexcept {
		swap(VEG_FWD(u), (VEG_FWD(v)));
	}
};
struct mov_fn_swap {
	template <typename U, typename V>
	using type = void;

	template <typename U>
	VEG_INLINE static VEG_CPP14(constexpr) void apply(U& u, U& v) noexcept {
		auto tmp = static_cast<U&&>(u);
		u = static_cast<U&&>(v);
		v = static_cast<U&&>(tmp);
	}
};

template <typename U, typename V>
struct has_adl_swap
		: bool_constant<VEG_CONCEPT(detected<adl_fn_swap::type, U, V>)>,
			adl_fn_swap {};

template <typename U, typename V>
struct has_mov_swap : meta::false_type {};
template <typename U>
struct has_mov_swap<U&, U&> : bool_constant<
																	VEG_CONCEPT(move_constructible<U>) &&
																	VEG_CONCEPT(move_assignable<U>)>,
															mov_fn_swap {};

template <typename U, typename V>
struct swap_impl : meta::disjunction<has_adl_swap<U, V>, has_mov_swap<U, V>> {};

template <typename U, typename V>
using nothrow_swap_expr = enable_if_t<VEG_IS_NOEXCEPT((swap_impl<U, V>::apply(
		VEG_DECLVAL_NOEXCEPT(U &&), VEG_DECLVAL_NOEXCEPT(V&&))))>;

template <typename U, typename V>
using adl_swap_expr = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));
} // namespace meta_
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		(typename U, typename V),
		adl_swappable,
		VEG_CONCEPT(detected<internal::meta_::adl_swap_expr, U&&, V&&>));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename U, typename V),
		move_swappable,
		((, same<U, V>),
     (, lvalue_reference<U>),
     (, move_constructible<meta::unref_t<U>>),
     (, move_assignable<meta::unref_t<U>>)));

VEG_DEF_CONCEPT_DISJUNCTION(
		(typename U, typename V),
		swappable,
		((, adl_swappable<U, V>), (, move_swappable<U, V>)));

VEG_DEF_CONCEPT(
		(typename U, typename V),
		nothrow_swappable,
		VEG_CONCEPT(detected<internal::meta_::nothrow_swap_expr, U&&, V&&>));
} // namespace concepts

namespace nb {
struct swap {
	VEG_TEMPLATE(
			(typename U, typename V),
			requires(VEG_CONCEPT(swappable<U&&, V&&>)),
			VEG_INLINE VEG_CPP14(constexpr) void
			operator(),
			(u, U&&),
			(v, V&&))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_swappable<U&&, V&&>)) {
		internal::meta_::swap_impl<U&&, V&&>::apply(VEG_FWD(u), VEG_FWD(v));
	}
};
} // namespace nb
VEG_NIEBLOID(swap);

namespace cpo {
template <typename U>
struct is_trivially_swappable : meta::false_type {};

template <typename T>
struct is_trivially_swappable<T&>
		: meta::bool_constant<(
					VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(trivially_move_assignable<T>) &&
					VEG_CONCEPT(trivially_move_constructible<T>) &&
					!VEG_CONCEPT(adl_swappable<T&, T&>))> {};
} // namespace cpo
namespace concepts {
VEG_DEF_CONCEPT(
		typename T, trivially_swappable, cpo::is_trivially_swappable<T>::value);
} // namespace concepts
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_SWAP_HPP_CTDUPQCUS */
