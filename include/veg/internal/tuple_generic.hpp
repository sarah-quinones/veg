#ifndef __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/meta_int_fix.hpp"
#include "veg/internal/storage.hpp"
#include "veg/internal/cmp.hpp"
#include <utility>
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
template <typename... Ts>
struct tuple;

namespace internal {
namespace tuple {

template <typename T, typename U>
struct trivially_swappable : meta::false_type {};
template <typename... Ts>
struct trivially_swappable<veg::tuple<Ts...>&, veg::tuple<Ts...>&>
		: meta::bool_constant<meta::all_of(
					{!VEG_CONCEPT(reference<Ts>) &&
           VEG_CONCEPT(trivially_swappable<Ts&>)...})> {};

} // namespace tuple
} // namespace internal

namespace meta {
template <typename... Ts>
struct is_trivially_swappable<tuple<Ts...>&>
		: veg::internal::tuple::trivially_swappable<tuple<Ts...>&, tuple<Ts...>&> {
};
} // namespace meta

namespace internal {
namespace tuple {

template <usize I, typename T>
struct fake_leaf {};

template <typename ISeq, typename... Ts>
struct fake_impl;
template <usize... Is, typename... Ts>
struct fake_impl<meta::index_sequence<Is...>, Ts...> : fake_leaf<Is, Ts>... {};

template <usize I, typename T>
auto get_type(fake_leaf<I, T> const&) noexcept -> T;

template <usize I>
struct pack_ith_elem {
	template <typename... Ts>
	using type = decltype(tuple::get_type<I>(
			VEG_DECLVAL(fake_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>)));
};

} // namespace tuple
} // namespace internal

template <usize I, typename... Ts>
using ith = typename internal::tuple::pack_ith_elem<I>::template type<Ts...>;

namespace internal {
namespace tuple {

struct hidden_tag0 {};
struct hidden_tag1 {};
struct hidden_tag2 {};

template <typename T>
void get() = delete;
template <usize I, typename T>
HEDLEY_ALWAYS_INLINE auto adl_get(T&& arg) VEG_DEDUCE_RET(get<I>(VEG_FWD(arg)));

template <usize I, typename T>
struct tuple_leaf {
	T inner;
};

template <typename ISeq, typename... Ts>
struct tuple_impl;

template <usize... Is, typename... Ts>
struct tuple_impl<meta::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {
	constexpr tuple_impl() = default;
	HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
			hidden_tag0 /*unused*/, Ts&&... args)

			noexcept(meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Ts&&>)...}))
			: tuple_leaf<Is, Ts>{Ts(VEG_FWD(args))}... {}

	template <typename... Fn>
	HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
			hidden_tag1 /*unused*/,
			Fn&&... fns) noexcept(meta::
	                              all_of(
																		{VEG_CONCEPT(nothrow_invocable<Fn&&>)...}))
			: tuple_leaf<Is, Ts>{Ts(VEG_FWD(fns)())}... {}

	template <meta::category_e C, typename... Us>
	HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
			hidden_tag2 /*unused*/,
			std::integral_constant<meta::category_e, C>* /*unused*/,
			tuple_impl<meta::index_sequence<Is...>, Us...> const& tup)

			noexcept(meta::all_of({VEG_CONCEPT(
					nothrow_constructible<Ts, meta::apply_category_t<C, Us>&&>)...}))
			: tuple_leaf<Is, Ts>{
						Ts(const_cast<meta::apply_category_t<C, Us>&&> /* NOLINT */ (
								static_cast<tuple_leaf<Is, Us> const&>(tup).inner))}... {}
};

template <usize I, typename T>
auto get_impl(tuple_leaf<I, T> const& leaf) noexcept -> T& {
	return const_cast<T&> /* NOLINT */ (leaf.inner);
}

template <typename... Ts>
struct binop_ftor {
	template <typename Fn, bool NoExcept, typename... Us>
	static VEG_CPP14(constexpr) void apply(Ts&&... ts, Us&&... us) noexcept(
			NoExcept) {
		(void)meta::internal::empty_arr{
				meta::internal::empty{},
				(Fn::apply(ts, us), meta::internal::empty{})...};
	}
};

template <
		typename Fn,
		bool NoExcept,
		typename... Actual_Ts,
		typename... Actual_Us,
		usize... Is,
		typename... Ts,
		typename... Us>
HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void binop_apply(
		veg::tuple<Actual_Ts...>* /*unused*/,
		veg::tuple<Actual_Us...>* /*unused*/,
		tuple_impl<meta::index_sequence<Is...>, Ts...> const& t,
		tuple_impl<meta::index_sequence<Is...>, Us...> const& u)

		noexcept(NoExcept) {
	binop_ftor<Actual_Ts&&...>::template apply<Fn, NoExcept>(

			const_cast<Actual_Ts&&> /* NOLINT */ (
					static_cast<tuple_leaf<Is, Ts> const&>(t).inner)...,
			const_cast<Actual_Us&&> /* NOLINT */ (
					static_cast<tuple_leaf<Is, Us> const&>(u).inner)...

	);
}

template <bool Cvt>
struct assign_impl {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(
			L& l, R&& r) noexcept {
		l = static_cast<L>(VEG_FWD(r));
	}
};
template <>
struct assign_impl<false> {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(
			L&& l, R&& r) noexcept(VEG_CONCEPT(nothrow_assignable<L&&, R&&>)) {
		VEG_FWD(l) = VEG_FWD(r);
	}
};

struct assign_ftor {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(
			L&& l, R&& r) noexcept(VEG_CONCEPT(nothrow_assignable<L&&, R&&>)) {
		assign_impl<
				VEG_CONCEPT(arithmetic<meta::uncvref_t<L>>) &&
				VEG_CONCEPT(
						arithmetic<meta::uncvref_t<R>>)>::apply(VEG_FWD(l), VEG_FWD(r));
	}
};
struct swap_ftor {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(
			L&& l, R&& r) noexcept(VEG_CONCEPT(nothrow_swappable<L&&, R&&>)) {
		veg::swap(VEG_FWD(l), VEG_FWD(r));
	}
};

#if __cplusplus >= 201703L
template <typename... Ts>
struct cmp_eq_impl {
	template <typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	apply(Ts const&... ts, Us const&... us) noexcept -> bool {
		return (veg::cmp_equal(ts, us) && ... && true);
	}
};
#else
template <typename... Ts>
struct cmp_eq_impl;
template <typename T0, typename... Ts>
struct cmp_eq_impl<T0, Ts...> {
	template <typename U0, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	apply(T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) noexcept
			-> bool {
		return (veg::cmp_equal(t0, u0) && cmp_eq_impl<Ts...>::apply(ts..., us...));
	}
};
template <>
struct cmp_eq_impl<> {
	HEDLEY_ALWAYS_INLINE static constexpr auto apply() noexcept -> bool {
		return true;
	}
};
#endif
// TODO: three-way comparison customization point

struct cmp_impl {
	template <usize... Is, typename... Ts, typename... Us>
	static HEDLEY_ALWAYS_INLINE constexpr auto apply(
			tuple_impl<meta::index_sequence<Is...>, Ts...> const& lhs,
			tuple_impl<meta::index_sequence<Is...>, Us...> const& rhs) noexcept
			-> bool {

		return cmp_eq_impl<meta::uncvref_t<Ts>...>::apply(

				static_cast<tuple_leaf<Is, Ts> const&>(lhs).inner...,
				static_cast<tuple_leaf<Is, Us> const&>(rhs).inner...

		);
	}
};

template <
		bool NoExcept,
		meta::category_e CL,
		meta::category_e CR,
		usize... Is,
		typename... Ts,
		typename... Us>
HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap_impl(
		tuple_impl<meta::index_sequence<Is...>, Ts...> const& lhs,
		tuple_impl<meta::index_sequence<Is...>, Us...> const& rhs)

		noexcept(NoExcept) {

	tuple::binop_apply<swap_ftor, NoExcept>(
			static_cast<veg::tuple<meta::apply_category_t<CL, Ts>&&...>*>(nullptr),
			static_cast<veg::tuple<meta::apply_category_t<CR, Us>&&...>*>(nullptr),
			lhs,
			rhs);
}

template <typename... Ts>
auto detect_tuple(veg::tuple<Ts...> const&) noexcept -> veg::tuple<Ts...>;
template <typename T>
using detect_tuple_t = decltype(detect_tuple(VEG_DECLVAL(T &&)));
template <typename T>
struct is_tuple : meta::is_detected<detect_tuple_t, T> {};

template <typename... Ts>
struct is_tuple<veg::tuple<Ts...>> : meta::true_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...> const> : meta::true_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...>&> : meta::true_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...> const&> : meta::true_type {};

template <typename T, bool = is_tuple<T>::value>
struct bare_tuple_type_impl {};
template <typename T>
struct bare_tuple_type_impl<T, true> {
	using type = detect_tuple_t<T>;
};

template <typename T, bool = is_tuple<T>::value>
struct tuple_type_impl {};
template <typename T>
struct tuple_type_impl<T, true> {
	using type = meta::collapse_category_t<detect_tuple_t<T>, T>;
};

template <typename T>
struct base_tuple : tuple_type_impl<T> {};
template <typename T>
struct bare_base_tuple : bare_tuple_type_impl<T> {};

template <typename T>
using base_tuple_t = typename base_tuple<T>::type;
template <typename T>
using bare_base_tuple_t = typename bare_base_tuple<T>::type;

template <typename T>
HEDLEY_ALWAYS_INLINE constexpr auto get_inner(T&& tup) noexcept
		-> decltype((VEG_FWD(tup).m_impl))&& {
	return VEG_FWD(tup).m_impl;
}

// template <usize I, typename T>
// HEDLEY_ALWAYS_INLINE constexpr auto access_impl(T&& tup)
// 		VEG_DEDUCE_RET(tuple::get_impl<meta::value_category<T>::value, I>(
// 				tuple::get_inner(VEG_FWD(tup))));

template <bool Trivial>
struct impl {
	template <bool NoExcept, typename Tup>
	static HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void apply(Tup& ts, Tup& us) noexcept {
		static_assert(NoExcept, "no");
		Tup tmp = ts;
		ts = us;
		us = tmp;
	}
};
template <>
struct impl<false> {
	template <bool NoExcept, typename Tup_Lhs, typename Tup_Rhs>
	static HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void apply(
			Tup_Lhs&& lhs, Tup_Rhs&& rhs) noexcept(NoExcept) {
		return internal::tuple::swap_impl<
				NoExcept,
				meta::value_category<Tup_Lhs&&>::value,
				meta::value_category<Tup_Rhs&&>::value>(
				tuple::get_inner(lhs), tuple::get_inner(rhs));
	}
};

template <
		typename Tup_L,
		typename Tup_R,
		meta::category_e CL,
		meta::category_e CR>
struct tup_swappable : meta::false_type {};

template <
		typename... Ts,
		typename... Us,
		meta::category_e CL,
		meta::category_e CR>
struct tup_swappable<veg::tuple<Ts...>, veg::tuple<Us...>, CL, CR>
		: meta::bool_constant<meta::all_of(
					{VEG_CONCEPT(swappable<                      //
											 meta::apply_category_t<CL, Ts>, //
											 meta::apply_category_t<CR, Us>  //
											 >)...})> {};

template <
		typename Tup_L,
		typename Tup_R,
		meta::category_e CL,
		meta::category_e CR>
struct nothrow_tup_swappable : meta::false_type {};

template <
		typename... Ts,
		typename... Us,
		meta::category_e CL,
		meta::category_e CR>
struct nothrow_tup_swappable<veg::tuple<Ts...>, veg::tuple<Us...>, CL, CR>
		: meta::bool_constant<meta::all_of(
					{VEG_CONCEPT(nothrow_swappable<              //
											 meta::apply_category_t<CL, Ts>, //
											 meta::apply_category_t<CR, Us>  //
											 >)...})> {};

namespace adl {
template <typename... Ts>
struct tuple_base {};

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, veg::tuple<Ts...>&))
noexcept -> ith<I, Ts...>& {
	return tuple::get_impl<I>(tuple::get_inner(tup));
}
VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, veg::tuple<Ts...> const&))
noexcept -> ith<I, Ts...> const& {
	return tuple::get_impl<I>(tuple::get_inner(tup));
}
VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, veg::tuple<Ts...>&&))
noexcept -> ith<I, Ts...>&& {
	return static_cast<ith<I, Ts...>&&>(
			tuple::get_impl<I>(tuple::get_inner(tup)));
}
VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr void get,
		(tup, veg::tuple<Ts...> const&&))
noexcept = delete;

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				!VEG_CONCEPT(reference<Ts>) && !VEG_CONCEPT(reference<Us>) &&
				VEG_CONCEPT(swappable<Ts&, Us&>))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(ts, veg::tuple<Ts...>&),
		(us, veg::tuple<Us...>&))
noexcept(meta::all_of({VEG_CONCEPT(nothrow_swappable<Ts&, Us&>)...})) {
	internal::tuple::impl<meta::all_of(
			{(VEG_CONCEPT(same<Ts, Us>) &&
	      VEG_CONCEPT(trivially_swappable<Ts&>))...})>::
			template apply<(
					meta::all_of({VEG_CONCEPT(nothrow_swappable<Ts&, Us&>)...}))>(ts, us);
}

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts const&&, Us const&&>))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(ts, veg::tuple<Ts...> const&&),
		(us, veg::tuple<Us...> const&&))

noexcept(
		meta::all_of({VEG_CONCEPT(nothrow_swappable<Ts const&&, Us const&&>)...})) {
	internal::tuple::impl<false>::template apply<(meta::all_of(
			{VEG_CONCEPT(nothrow_swappable<Ts const&&, Us const&&>)...}))>(
			VEG_FWD(ts), VEG_FWD(us));
}
VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				!VEG_CONCEPT(reference<Ts>) &&
				VEG_CONCEPT(swappable<Ts&, Us const&&>))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(ts, veg::tuple<Ts...>&),
		(us, veg::tuple<Us...> const&&))

noexcept(meta::all_of({VEG_CONCEPT(nothrow_swappable<Ts&, Us const&&>)...})) {
	internal::tuple::impl<false>::template apply<(meta::all_of({VEG_CONCEPT(
			nothrow_swappable<Ts&, Us const&&>)...}))>(VEG_FWD(ts), VEG_FWD(us));
}
VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				!VEG_CONCEPT(reference<Us>) &&
				VEG_CONCEPT(swappable<Ts const&&, Us&>))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(ts, veg::tuple<Ts...> const&&),
		(us, veg::tuple<Us...>&))
noexcept(meta::all_of({VEG_CONCEPT(nothrow_swappable<Ts const&&, Us&>)...})) {
	internal::tuple::impl<false>::template apply<(meta::all_of({VEG_CONCEPT(
			nothrow_swappable<Ts const&&, Us&>)...}))>(VEG_FWD(ts), VEG_FWD(us));
}

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires VEG_ALL_OF(VEG_CONCEPT(equality_comparable_with<Ts, Us>)),
		HEDLEY_ALWAYS_INLINE constexpr auto
		operator==,
		(lhs, veg::tuple<Ts...> const&),
		(rhs, veg::tuple<Us...> const&))
noexcept -> bool {
	return cmp_impl::apply(tuple::get_inner(lhs), tuple::get_inner(rhs));
}
VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires VEG_ALL_OF(VEG_CONCEPT(equality_comparable_with<Ts, Us>)),
		HEDLEY_ALWAYS_INLINE constexpr auto
		operator!=,
		(lhs, veg::tuple<Ts...> const&),
		(rhs, veg::tuple<Us...> const&))
noexcept -> bool {
	return !cmp_impl::apply(tuple::get_inner(lhs), tuple::get_inner(rhs));
}

} // namespace adl

template <bool Movable, typename... Ts>
struct tuple_ctor_base {
	constexpr tuple_ctor_base() = default;

	HEDLEY_ALWAYS_INLINE constexpr explicit tuple_ctor_base(Ts... args) noexcept(
			meta::all_of({VEG_CONCEPT(nothrow_move_constructible<Ts>)...}))
			: m_impl(hidden_tag0{}, VEG_FWD(args)...) {}

	HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
			(elems_t /*tag*/, Ts... args) noexcept(
					meta::all_of({VEG_CONCEPT(nothrow_move_constructible<Ts>)...}))
			: m_impl(hidden_tag0{}, VEG_FWD(args)...) {}

	VEG_TEMPLATE(
			(typename... Fns),
			requires VEG_ALL_OF(
					VEG_CONCEPT(invocable<Fns&&>) && //
					VEG_CONCEPT(same<Ts, meta::invoke_result_t<Fns&&>>)),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			(/*tag*/, inplace_t),
			(... fns, Fns&&))
	noexcept(meta::all_of({VEG_CONCEPT(nothrow_invocable<Fns&&>)...}))
			: m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us&&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>)),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...>&&)),
			noexcept(meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us&&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<std::integral_constant<
							meta::category_e,
							meta::category_e::ref_mov>*>(nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF((VEG_CONCEPT(constructible<Ts, Us&>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...>&)),
			noexcept(meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<std::integral_constant<
							meta::category_e,
							meta::category_e::ref_mut>*>(nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us const&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us const&>)),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...> const&)),
			noexcept(
					meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us const&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<
							std::integral_constant<meta::category_e, meta::category_e::ref>*>(
							nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_DEFAULT_CTOR_ASSIGN(tuple_ctor_base);

private:
	internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
			m_impl;

	template <typename T>
	friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
			-> decltype((VEG_FWD(tup).m_impl))&&;
};

template <>
struct tuple_ctor_base<true> {
	constexpr tuple_ctor_base() = default;

	HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
			(elems_t /*tag*/) noexcept
			: tuple_ctor_base() {}
	HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
			(inplace_t /*tag*/) noexcept
			: tuple_ctor_base() {}

	VEG_DEFAULT_CTOR_ASSIGN(tuple_ctor_base);

private:
	internal::tuple::tuple_impl<meta::index_sequence<>> m_impl;

	template <typename T>
	friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
			-> decltype((VEG_FWD(tup).m_impl))&&;
};

template <typename... Ts>
struct tuple_ctor_base<false, Ts...> {
	constexpr tuple_ctor_base() = default;

	VEG_TEMPLATE(
			(typename... Fns),
			requires VEG_ALL_OF(
					(VEG_CONCEPT(invocable<Fns&&>) && //
	         VEG_CONCEPT(same<Ts, meta::invoke_result_t<Fns&&>>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			(/*tag*/, inplace_t),
			(... fns, Fns&&))
	noexcept(meta::all_of({VEG_CONCEPT(nothrow_invocable<Fns&&>)...}))
			: m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us&&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>)),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...>&&)),
			noexcept(meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us&&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<std::integral_constant<
							meta::category_e,
							meta::category_e::ref_mov>*>(nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF((VEG_CONCEPT(constructible<Ts, Us&>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...>&)),
			noexcept(meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<std::integral_constant<
							meta::category_e,
							meta::category_e::ref_mut>*>(nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_TEMPLATE_EXPLICIT(
			!VEG_ALL_OF(VEG_CONCEPT(convertible<Us const&, Ts>)),
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us const&>)),
			HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
			((tup, veg::tuple<Us...> const&)),
			noexcept(
					meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Us const&>)...}))
			: m_impl(
					hidden_tag2{},
					static_cast<
							std::integral_constant<meta::category_e, meta::category_e::ref>*>(
							nullptr),
					tuple::get_inner(VEG_FWD(tup))){})

	VEG_DEFAULT_CTOR_ASSIGN(tuple_ctor_base);

private:
	internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
			m_impl;

	template <typename T>
	friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
			-> decltype((VEG_FWD(tup).m_impl))&&;
};

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple : internal::tuple::tuple_ctor_base<
									 meta::all_of({VEG_CONCEPT(move_constructible<Ts>)...}),
									 Ts...>,
							 internal::tuple::adl::tuple_base<Ts...> {
	using base = internal::tuple::tuple_ctor_base<
			meta::all_of({VEG_CONCEPT(move_constructible<Ts>)...}),
			Ts...>;
	using base::base;
	using base::operator=;

	VEG_DEFAULT_CTOR_ASSIGN(tuple);

	template <typename... Us>
	tuple(veg::tuple<Us...> const&& tup) = delete;

	template <i64 I>
	void operator[](fix<I> /*arg*/) const&& = delete;

	VEG_TEMPLATE(
			(i64 I),
			requires(
					I < sizeof...(Ts) && (I >= 0) &&
					VEG_CONCEPT(move_constructible<ith<usize{I}, Ts...>>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/,
	     fix<I>)) && noexcept(VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
			-> ith<usize{I}, Ts...> {
		return static_cast<ith<usize{I}, Ts...>&&>(
				internal::tuple::get_impl<I>(internal::tuple::get_inner(*this)));
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, fix<I>)) & noexcept -> ith<usize{I}, Ts...>& {
		return internal::tuple::get_impl<I>(internal::tuple::get_inner(*this));
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, fix<I>))
	const& noexcept->ith<usize{I}, Ts...> const& {
		return internal::tuple::get_impl<I>(internal::tuple::get_inner(*this));
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires VEG_ALL_OF(
					!VEG_CONCEPT(reference<Ts>) &&
					VEG_CONCEPT(assignable<Ts&, Us const&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, veg::tuple<Us...> const&)) &

			noexcept(meta::all_of(
					{VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>)...})) -> tuple& {

		internal::tuple::binop_apply<
				internal::tuple::assign_ftor,
				meta::all_of({VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>)...})>(

				static_cast<tuple<Ts&...>*>(nullptr),
				static_cast<tuple<Us const&...>*>(nullptr),
				internal::tuple::get_inner(*this),
				internal::tuple::get_inner(rhs)

		);

		return *this;
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires VEG_ALL_OF(
					!VEG_CONCEPT(reference<Ts>) && VEG_CONCEPT(assignable<Ts&, Us&&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, veg::tuple<Us...>&&)) &

			noexcept(meta::all_of({VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>)...}))
					-> tuple& {

		internal::tuple::binop_apply<
				internal::tuple::assign_ftor,
				meta::all_of({VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>)...})>(

				static_cast<tuple<Ts&...>*>(nullptr),
				static_cast<tuple<Us&&...>*>(nullptr),
				internal::tuple::get_inner(*this),
				internal::tuple::get_inner(rhs)

		);

		return *this;
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us const&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, veg::tuple<Us...> const&))
	const&&

			noexcept(meta::all_of(
									 {VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>)...}))
					->tuple const&& {

		internal::tuple::binop_apply<
				internal::tuple::assign_ftor,
				meta::all_of(
						{VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>)...})>(

				static_cast<tuple<Ts const&&...>*>(nullptr),
				static_cast<tuple<Us const&...>*>(nullptr),
				internal::tuple::get_inner(*this),
				internal::tuple::get_inner(rhs)

		);

		return static_cast<tuple const&&>(*this);
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us&&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, veg::tuple<Us...>&&))
	const&&

			noexcept(
					meta::all_of({VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>)...}))
					->tuple const&& {

		internal::tuple::binop_apply<
				internal::tuple::assign_ftor,
				meta::all_of({VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>)...})>(

				static_cast<tuple<Ts const&&...>*>(nullptr),
				static_cast<tuple<Us&&...>*>(nullptr),
				internal::tuple::get_inner(*this),
				internal::tuple::get_inner(rhs)

		);

		return static_cast<tuple const&&>(*this);
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() & noexcept
			-> tuple<Ts&...> {
		return tuple::as_ref_impl<Ts&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() const& noexcept
			-> tuple<Ts const&...> {
		return tuple::as_ref_impl<Ts const&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() && noexcept
			-> tuple<Ts&&...> {
		return tuple::as_ref_impl<Ts&&...>(
				static_cast<tuple&&>(*this),
				meta::make_index_sequence<sizeof...(Ts)>{});
	}

private:
	template <typename... Us, usize... Is>
	static constexpr auto HEDLEY_ALWAYS_INLINE as_ref_impl(
			tuple<Ts...> const& self, meta::index_sequence<Is...> /*seq*/) noexcept
			-> veg::tuple<Us...> {
		return {
				elems,

				const_cast<Us> /* NOLINT */ (
						static_cast<internal::tuple::tuple_leaf<Is, Ts> const&>(
								internal::tuple::get_inner(self))
								.inner)

						...};
	}
};

VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
