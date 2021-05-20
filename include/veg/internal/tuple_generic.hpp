#ifndef VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/integer_seq.hpp"
#include "veg/type_traits/value_category.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/util/swap.hpp"
#include "veg/util/compare.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename... Ts>
struct Tuple;

namespace internal {
namespace tup_ {

template <usize I, typename T>
struct TupleLeaf {
	T leaf;
};

template <typename ISeq, typename... Ts>
struct IndexedTuple;

template <usize... Is, typename... Ts>
struct IndexedTuple<meta::index_sequence<Is...>, Ts...> : TupleLeaf<Is, Ts>... {

	IndexedTuple() = default;

	VEG_INLINE constexpr IndexedTuple(Direct /*tag*/, Ts... args)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
			: TupleLeaf<Is, Ts>{VEG_FWD(args)}... {};

	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(invocable_r<Fns, Ts>))),
			VEG_INLINE constexpr IndexedTuple,
			(/*tag*/, InPlace),
			(... fns, Fns))
	VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Ts>)))
			: TupleLeaf<Is, Ts>{VEG_FWD(fns)()}... {}

	VEG_EXPLICIT_COPY(IndexedTuple);

	/// returns a tuple of references `Ts&...`
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() &
			VEG_NOEXCEPT -> Tuple<Ts&...> {
		return {Direct{}, static_cast<TupleLeaf<Is, Ts>&>(*this).leaf...};
	}
	/// returns a tuple of references `Ts const&...`
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Tuple<Ts const&...> {
		return {Direct{}, static_cast<TupleLeaf<Is, Ts> const&>(*this).leaf...};
	}
	/// returns a tuple of references `Ts&&...`
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() &&
			VEG_NOEXCEPT -> Tuple<Ts&&...> {
		return {
				Direct{},
				static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf)...};
	}

	template <i64 I>
	void operator[](Fix<I> /*arg*/) const&& = delete;

	VEG_TEMPLATE(
			(i64 I),
			requires(
					I < sizeof...(Ts) && (I >= 0) &&
					VEG_CONCEPT(move_constructible<ith<usize{I}, Ts...>>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &&
			VEG_NOEXCEPT -> ith<usize{I}, Ts...> {
		return static_cast<ith<usize{I}, Ts...>&&>(
				static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>>&&>(*this).leaf);
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &
			VEG_NOEXCEPT -> ith<usize{I}, Ts...>& {
		return static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>>&>(*this).leaf;
	}

	/// returns a const reference to the ith element
	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>))
	const & VEG_NOEXCEPT->ith<usize{I}, Ts...> const& {
		return static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>> const&>(*this)
		    .leaf;
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires(
					VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>)) &&
					VEG_ALL_OF(VEG_CONCEPT(assignable<Ts&, Us&&>))),
			VEG_INLINE VEG_CPP14(constexpr) void // NOLINT
			operator=,
			(rhs, Tuple<Us...>&&)) &
			VEG_NOEXCEPT_IF(
					(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>)))) {
		VEG_EVAL_ALL(
				(static_cast<TupleLeaf<Is, Ts>&>(*this).leaf =
		         static_cast<Us&&>(static_cast<TupleLeaf<Is, Us>&&>(rhs).leaf)));
	}
	VEG_TEMPLATE(
			(typename... Us),
			requires(
					VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>)) &&
					VEG_ALL_OF(VEG_CONCEPT(assignable<Ts&, Us const&>))),
			VEG_INLINE VEG_CPP14(constexpr) void // NOLINT
			operator=,
			(rhs, Tuple<Us...> const&)) &
			VEG_NOEXCEPT_IF(
					(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>)))) {
		VEG_EVAL_ALL(
				(static_cast<TupleLeaf<Is, Ts>&>(*this).leaf =
		         static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf));
	}

	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us&&>))),
			VEG_INLINE VEG_CPP14(constexpr) void // NOLINT
			operator=,
			(rhs, Tuple<Us...>&&))
	const&& VEG_NOEXCEPT_IF(
			(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>)))) {
		VEG_EVAL_ALL(
				static_cast<Ts const&&>(
						static_cast<TupleLeaf<Is, Ts> const&>(*this).leaf) =
						static_cast<Us&&>(static_cast<TupleLeaf<Is, Us>&&>(rhs).leaf));
	}
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us const&>))),
			VEG_INLINE VEG_CPP14(constexpr) void // NOLINT
			operator=,
			(rhs, Tuple<Us...> const&))
	const&& VEG_NOEXCEPT_IF(
			(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>)))) {
		VEG_EVAL_ALL(
				static_cast<Ts const&&>(
						static_cast<TupleLeaf<Is, Ts> const&>(*this).leaf) =
						static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf);
	}
};

#if __cplusplus >= 201703L
template <typename... Ts>
struct CmpImpl {
	template <typename... Us>
	VEG_INLINE static constexpr auto
	eq(Ts const&... ts, Us const&... us) VEG_NOEXCEPT -> bool {
		return ((ts == us) && ... && true);
	}

	template <typename Ret, typename... Us>
	VEG_INLINE static constexpr auto
	tway(Ts const&... ts, Us const&... us) VEG_NOEXCEPT -> Ret {

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
struct CmpImpl;
template <typename T0, typename... Ts>
struct CmpImpl<T0, Ts...> {

	template <typename U0, typename... Us>
	VEG_INLINE static constexpr auto
	eq(T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) VEG_NOEXCEPT
			-> bool {
		return (t0 == u0) && CmpImpl<Ts...>::eq(ts..., us...);
	}

	template <typename Ret, typename U0, typename... Us>
	VEG_INLINE static constexpr auto tway(
			T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) VEG_NOEXCEPT
			-> Ret {
		return CmpImpl::template tway2<Ret>(
				Ret(veg::cmp::synth_three_way(t0, u0)), ts..., us...);
	}

	template <typename Ret, typename... Us>
	VEG_INLINE static constexpr auto
	tway2(Ret res, Ts const&... ts, Us const&... us) VEG_NOEXCEPT -> Ret {
		return (res != 0) ? res
		                  : CmpImpl<Ts...>::template tway<Ret>(
														VEG_FWD(ts)..., VEG_FWD(us)...);
	}
};
template <>
struct CmpImpl<> {
	VEG_INLINE static constexpr auto eq() VEG_NOEXCEPT -> bool { return true; }
	template <typename>
	VEG_INLINE static constexpr auto tway() VEG_NOEXCEPT -> cmp::strong_ordering {
		return cmp::strong_ordering::equal;
	}
};

#endif

struct cmp_impl {
	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto
	eq(IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
	   IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs) VEG_NOEXCEPT
			-> bool {

		return CmpImpl<Ts const&...>::eq(

				static_cast<TupleLeaf<Is, Ts> const&>(lhs).leaf...,
				static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf...

		);
	}

	template <typename Ret, usize... Is, typename... Ts, typename... Us>
	static constexpr auto tway(
			IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
			IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs) VEG_NOEXCEPT
			-> Ret {

		return CmpImpl<Ts const&...>::template tway<Ret>(

				static_cast<TupleLeaf<Is, Ts> const&>(lhs).leaf...,
				static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf...

		);
	}
};

template <bool Trivial>
struct SwapImpl {
	template <typename Tup>
	static VEG_INLINE
	VEG_CPP14(constexpr) void apply(Tup& ts, Tup& us) VEG_NOEXCEPT {
		Tup tmp = static_cast<Tup&&>(ts);
		ts = static_cast<Tup&&>(us);
		us = static_cast<Tup&&>(tmp);
	}
};
template <>
struct SwapImpl<false> {
	template <usize... Is, typename... Ts, typename... Us>
	static VEG_INLINE VEG_CPP14(constexpr) void apply(
			IndexedTuple<meta::index_sequence<Is...>, Ts...>& lhs,
			IndexedTuple<meta::index_sequence<Is...>, Us...>& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us&>))) {
		(void)EmptyArr{
				Empty{},
				(veg::swap( //
						 static_cast<Ts&>(static_cast<TupleLeaf<Is, Ts>&>(lhs).leaf),
						 static_cast<Us&>(static_cast<TupleLeaf<Is, Us>&>(rhs).leaf)),
		     Empty{})...};
	}
};

namespace adl {
template <typename... Ts>
struct tuple_base {};

} // namespace adl
} // namespace tup_

} // namespace internal
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
