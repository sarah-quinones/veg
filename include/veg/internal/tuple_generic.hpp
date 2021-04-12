#ifndef VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/integer_seq.hpp"
#include "veg/type_traits/value_category.hpp"
#include "veg/internal/meta_int_fix.hpp"
#include "veg/util/swap.hpp"
#include "veg/util/compare.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
template <typename... Ts>
struct Tuple;

namespace internal {
namespace tup_ {

template <typename T, typename U>
struct trivially_swappable : meta::false_type {};
template <typename... Ts>
struct trivially_swappable<Tuple<Ts...>&, Tuple<Ts...>&>
		: meta::bool_constant<(VEG_ALL_OF(
					(!VEG_CONCEPT(reference<Ts>) &&
           VEG_CONCEPT(trivially_swappable<Ts&>))))> {};

} // namespace tup_
} // namespace internal

namespace internal {
namespace tup_ {

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
	using type = decltype(tup_::get_type<I>(
			VEG_DECLVAL(fake_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>)));
};

} // namespace tup_
} // namespace internal

template <usize I, typename... Ts>
using ith = typename internal::tup_::pack_ith_elem<I>::template type<Ts...>;

namespace internal {
namespace tup_ {

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
struct IndexedTuple;

template <usize... Is, typename... Ts>
struct IndexedTuple<meta::index_sequence<Is...>, Ts...>
		: tuple_leaf<Is, Ts>... {
	constexpr IndexedTuple() = default;

	template <typename... Us>
	HEDLEY_ALWAYS_INLINE constexpr IndexedTuple /* NOLINT */
			(Cvt /*unused*/, Us&&... args)          //
			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Ts&&>))))
			: tuple_leaf<Is, Ts>{Ts(VEG_FWD(args))}... {}

	template <meta::category_e C, typename... Us>
	HEDLEY_ALWAYS_INLINE constexpr explicit IndexedTuple(
			hidden_tag2 /*unused*/,
			meta::constant<meta::category_e, C>* /*unused*/,
			IndexedTuple<meta::index_sequence<Is...>, Us...> const& tup)

			noexcept((VEG_ALL_OF(VEG_CONCEPT(
					nothrow_constructible<Ts, meta::apply_category_t<C, Us>&&>))))
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
		(void)internal::EmptyArr{
				internal::Empty{}, (Fn::apply(ts, us), internal::Empty{})...};
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
		meta::type_sequence<Actual_Ts...> /*unused*/,
		meta::type_sequence<Actual_Us...> /*unused*/,
		IndexedTuple<meta::index_sequence<Is...>, Ts...> const& t,
		IndexedTuple<meta::index_sequence<Is...>, Us...> const& u)

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
struct impl_cmp {
	template <typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	eq(Ts const&... ts, Us const&... us) noexcept -> bool {
		return ((ts == us) && ... && true);
	}

	template <typename Ret, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	tway(Ts const&... ts, Us const&... us) noexcept -> Ret {

		bool found = false;
		Ret c = Ret::equivalent;

		(void)(((void)(c = veg::cmp::synth_three_way(ts, us)),
		  (static_cast<bool>(c != 0) ? void(found = true) : (void)0),
		  !found)

		 && ... && true);
		return c;
	}
};
#else

template <typename... Ts>
struct impl_cmp;
template <typename T0, typename... Ts>
struct impl_cmp<T0, Ts...> {

	template <typename U0, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	eq(T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) noexcept
			-> bool {
		return (t0 == u0) && impl_cmp<Ts...>::eq(ts..., us...);
	}

	template <typename Ret, typename U0, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	tway(T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) noexcept
			-> Ret {
		return impl_cmp::template tway2<Ret>(
				Ret(veg::cmp::synth_three_way(t0, u0)), ts..., us...);
	}

	template <typename Ret, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto
	tway2(Ret res, Ts const&... ts, Us const&... us) noexcept -> Ret {
		return (res != 0) ? res
											: impl_cmp<Ts...>::template tway<Ret>(
														VEG_FWD(ts)..., VEG_FWD(us)...);
	}
};
template <>
struct impl_cmp<> {
	HEDLEY_ALWAYS_INLINE static constexpr auto eq() noexcept -> bool {
		return true;
	}
	template <typename>
	HEDLEY_ALWAYS_INLINE static constexpr auto tway() noexcept
			-> cmp::strong_ordering {
		return cmp::strong_ordering::equal;
	}
};

#endif

struct cmp_impl {
	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto
	eq(IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
	   IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs) noexcept
			-> bool {

		return impl_cmp<Ts const&...>::eq(

				static_cast<tuple_leaf<Is, Ts> const&>(lhs).inner...,
				static_cast<tuple_leaf<Is, Us> const&>(rhs).inner...

		);
	}

	template <typename Ret, usize... Is, typename... Ts, typename... Us>
	static constexpr auto tway(
			IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
			IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs) noexcept
			-> Ret {

		return impl_cmp<Ts const&...>::template tway<Ret>(

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
		IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
		IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs)

		noexcept(NoExcept) {

	tup_::binop_apply<swap_ftor, NoExcept>(
			meta::type_sequence<meta::apply_category_t<CL, Ts>&&...>{},
			meta::type_sequence<meta::apply_category_t<CR, Us>&&...>{},
			lhs,
			rhs);
}

template <bool Trivial>
struct impl {
	template <bool NoExcept, typename Tup>
	static HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void apply(Tup& ts, Tup& us) noexcept {
		static_assert(NoExcept, "no");
		Tup tmp = static_cast<Tup&&>(ts);
		ts = static_cast<Tup&&>(us);
		us = static_cast<Tup&&>(tmp);
	}
};
template <>
struct impl<false> {
	template <bool NoExcept, typename Tup_Lhs, typename Tup_Rhs>
	static HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void apply(
			Tup_Lhs&& lhs, Tup_Rhs&& rhs) noexcept(NoExcept) {
		return internal::tup_::swap_impl<
				NoExcept,
				meta::value_category<Tup_Lhs&&>::value,
				meta::value_category<Tup_Rhs&&>::value>(lhs, rhs);
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
struct tup_swappable<Tuple<Ts...>, Tuple<Us...>, CL, CR>
		: meta::bool_constant<(
					VEG_ALL_OF(VEG_CONCEPT(swappable<                      //
																 meta::apply_category_t<CL, Ts>, //
																 meta::apply_category_t<CR, Us>  //
																 >)))> {};

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
struct nothrow_tup_swappable<Tuple<Ts...>, Tuple<Us...>, CL, CR>
		: meta::bool_constant<(
					VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<              //
																 meta::apply_category_t<CL, Ts>, //
																 meta::apply_category_t<CR, Us>  //
																 >)))> {};

template <typename... Actual_Ts, typename Fn, usize... Is, typename... Ts>
HEDLEY_ALWAYS_INLINE constexpr auto unpack_args_impl(
		meta::type_sequence<Actual_Ts...> /*tag*/,
		Fn&& fn,
		IndexedTuple<meta::index_sequence<Is...>, Ts...> const& args)

		noexcept(noexcept(VEG_FWD(fn)(VEG_DECLVAL_NOEXCEPT(Actual_Ts&&)...)))
				-> decltype(VEG_FWD(fn)(VEG_DECLVAL_NOEXCEPT(Actual_Ts &&)...)) {
	return VEG_FWD(fn)(
			((void)Is,
	     const_cast<Actual_Ts&&>(
					 static_cast<tuple_leaf<Is, Ts> const&>(args).inner))...);
}

namespace adl {
template <typename... Ts>
struct tuple_base {};

} // namespace adl
} // namespace tup_

template <typename... Ts>
using SimpleTuple =
		tup_::IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;
} // namespace internal

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
