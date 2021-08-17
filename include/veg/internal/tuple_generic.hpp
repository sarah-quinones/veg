#ifndef VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/integer_seq.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/util/swap.hpp"
#include "veg/util/compare.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/ref.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename... Ts>
struct Tuple;

namespace internal {

template <i64 I, typename Fn, typename Arg>
struct UnindexedFn {
	Fn&& fn;
	Arg&& arg;
	VEG_INLINE constexpr auto
	operator()() const&& VEG_DEDUCE_RET(VEG_FWD(fn)(Fix<I>{}, VEG_FWD(arg)));
};
template <typename Fn, typename Arg>
struct CurriedFn {
	Fn&& fn;
	Arg&& arg;
	VEG_INLINE constexpr auto
	operator()() const&& VEG_DEDUCE_RET(VEG_FWD(fn)(VEG_FWD(arg)));
};

} // namespace internal

namespace tuple {

template <
		usize I,
		typename T,
		bool = (VEG_CONCEPT(empty<T>) && !VEG_CONCEPT(final<T>))>
struct TupleLeaf;

template <usize I, typename T>
struct TupleLeaf<I, T, true> : T {
	VEG_INLINE constexpr auto leaf_get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(*this);
	}
};

template <usize I, typename T>
struct TupleLeaf<I, T, false> {
	T leaf;
	VEG_INLINE constexpr auto leaf_get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(leaf);
	}
};

template <typename ISeq, typename... Ts>
struct IndexedTuple;

template <usize... Is, typename... Ts>
struct IndexedTuple<meta::index_sequence<Is...>, Ts...> : TupleLeaf<Is, Ts>... {

	IndexedTuple() = default;

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_ALL_OF(VEG_CONCEPT(movable<Ts>))),
			VEG_INLINE constexpr IndexedTuple,
			((/*tag*/, Direct), (... args, Ts)),
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Ts>))))
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

	/// returns a tuple of references `Ts const&...`
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Tuple<Ref<Ts>...> {
		return {
				Direct{},
				Ref<Ts>{
						AsRef{},
						static_cast<Ts const&>(
								static_cast<TupleLeaf<Is, Ts> const&>(*this).leaf_get()),
				}...,
		};
	}
	/// returns a tuple of references `Ts&...`
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_mut() VEG_NOEXCEPT
			-> Tuple<RefMut<Ts>...> {
		return {
				Direct{},
				RefMut<Ts>{
						AsMut{},
						static_cast<TupleLeaf<Is, Ts>&>(*this).leaf_get(),
				}...,
		};
	}

	template <i64 I>
	void operator[](Fix<I> /*arg*/) const&& = delete;

	VEG_TEMPLATE(
			(i64 I),
			requires(
					I < sizeof...(Ts) && (I >= 0) &&
					VEG_CONCEPT(movable<ith<usize{I}, Ts...>>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &&
			VEG_NOEXCEPT -> ith<usize{I}, Ts...> {
		return static_cast<ith<usize{I}, Ts...>&&>(
				static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>>&&>(*this)
						.leaf_get());
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &
			VEG_NOEXCEPT -> ith<usize{I}, Ts...>& {
		return static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>>&>(*this)
		    .leaf_get();
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
		    .leaf_get();
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, Ts...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto unpack,
			(fn, Fn))
	&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Ts...>))->Ret {
		return VEG_FWD(fn)(static_cast<Ts&&>(
				static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf_get())...);
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_ALL_OF(VEG_CONCEPT(fn_mut<Fn, void, Ts>))),
			VEG_INLINE VEG_CPP14(constexpr) void for_each,
			(fn, Fn))
	&&VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut<Fn, void, Ts>))) {
		VEG_EVAL_ALL(fn(
				static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf_get())));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_ALL_OF(VEG_CONCEPT(fn_mut<Fn, void, Fix<i64{Is}>, Ts>))),
			VEG_INLINE VEG_CPP14(constexpr) void for_each_i,
			(fn, Fn))
	&&VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut<Fn, void, Fix<i64{Is}>, Ts>))) {
		VEG_EVAL_ALL(fn(
				Fix<i64{Is}>{},
				static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf_get())));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_ALL_OF(VEG_CONCEPT(fn_mut<
																 Fn,
																 meta::invoke_result_t<Fn&, Fix<i64{Is}>, Ts>,
																 Fix<i64{Is}>,
																 Ts>))),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto map_i,
			(fn, Fn))
	&&VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut<
															 Fn,
															 meta::invoke_result_t<Fn&, Fix<i64{Is}>, Ts>,
															 Fix<i64{Is}>,
															 Ts&&>)))
				->Tuple<meta::invoke_result_t<Fn&, Fix<i64{Is}>, Ts>...> {
		return {
				InPlace{},
				internal::UnindexedFn<i64{Is}, Fn&, Ts&&>{
						fn,
						static_cast<Ts&&>(
								static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf_get()),
				}...,
		};
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_ALL_OF(
					VEG_CONCEPT(fn_mut<Fn, meta::invoke_result_t<Fn&, Ts>, Ts>))),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto map,
			(fn, Fn))
	&&VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(
						nothrow_fn_mut<Fn, meta::invoke_result_t<Fn&, Ts>, Ts>)))
				->Tuple<meta::invoke_result_t<Fn&, Ts>...> {
		return {
				InPlace{},
				internal::CurriedFn<Fn&, Ts&&>{
						fn,
						static_cast<Ts&&>(
								static_cast<TupleLeaf<Is, Ts>&&>(*this).leaf_get()),
				}...,
		};
	}
};

} // namespace tuple

namespace cpo {
template <typename ISeq, typename... Ts>
struct TupleInterface;

template <usize... Is, typename... Ts>
struct TupleInterface<meta::index_sequence<Is...>, Ts...> {};

template <usize... Is, typename... Ts>
struct TupleInterface<meta::index_sequence<Is...>, RefMut<Ts>...> {
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_const() &&
			VEG_NOEXCEPT -> Tuple<Ref<Ts>...> {
		return {
				Direct{},
				{
						AsRef{},
						static_cast<Ts const&>(
								(static_cast<tuple::TupleLeaf<Is, RefMut<Ts>>&>(
										 static_cast<Tuple<RefMut<Ts>>&>(*this)))
										.leaf_get()
										.get()),
				}...,
		};
	}
};
} // namespace cpo

namespace internal {
namespace tup_ {

#if __cplusplus >= 201703L
template <typename... Ts>
struct CmpImpl {
	template <typename... Us>
	VEG_INLINE static constexpr auto eq(Ref<Ts>... ts, Ref<Us>... us) VEG_NOEXCEPT
			-> bool {
		return (cmp::eq(ts, us) && ... && true);
	}

	template <typename... Us>
	VEG_INLINE static constexpr auto
	tway(Ref<Ts>... ts, Ref<Us>... us) VEG_NOEXCEPT -> cmp::Ordering {

		bool found = false;
		auto c = cmp::Ordering::equal;

		(void)(((void)(c = cmp::cmp(ts, us)),
		  ((c != cmp::Ordering::equal) ? void(found = true) : (void)0),
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
	eq(Ref<T0> t0, Ref<Ts>... ts, Ref<U0> u0, Ref<Us>... us) VEG_NOEXCEPT
			-> bool {
		return cmp::eq(t0, u0) && CmpImpl<Ts...>::eq(ts..., us...);
	}

	template <typename U0, typename... Us>
	VEG_INLINE static constexpr auto
	tway(Ref<T0> t0, Ref<Ts>... ts, Ref<U0> u0, Ref<Us>... us) VEG_NOEXCEPT
			-> cmp::Ordering {
		return CmpImpl::tway2(cmp::cmp(t0, u0), ts..., us...);
	}

	template <typename... Us>
	VEG_INLINE static constexpr auto
	tway2(cmp::Ordering res, Ref<Ts>... ts, Ref<Us>... us) VEG_NOEXCEPT
			-> cmp::Ordering {
		return (res != cmp::Ordering::equal) ? res
		                                     : CmpImpl<Ts...>::tway(ts..., us...);
	}
};
template <>
struct CmpImpl<> {
	VEG_INLINE static constexpr auto eq() VEG_NOEXCEPT -> bool { return true; }
	VEG_INLINE static constexpr auto tway() VEG_NOEXCEPT -> cmp::Ordering {
		return cmp::Ordering::equal;
	}
};

#endif

struct cmp_impl {
	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto
	eq(tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
	   tuple::IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs)
			VEG_NOEXCEPT -> bool {
		return CmpImpl<Ts...>::eq(
				Ref<Ts>{
						as_ref,
						static_cast<Ts const&>(
								static_cast<tuple::TupleLeaf<Is, Ts> const&>(lhs).leaf_get()),
				}...,
				Ref<Us>{
						as_ref,
						static_cast<Ts const&>(
								static_cast<tuple::TupleLeaf<Is, Us> const&>(rhs).leaf_get()),
				}...);
	}

	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto tway(
			tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
			tuple::IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs)
			VEG_NOEXCEPT -> cmp::Ordering {
		return CmpImpl<Ts...>::tway(
				Ref<Ts>{
						as_ref,
						static_cast<Ts const&>(
								static_cast<tuple::TupleLeaf<Is, Ts> const&>(lhs).leaf_get()),
				}...,
				Ref<Us>{
						as_ref,
						static_cast<Us const&>(
								static_cast<tuple::TupleLeaf<Is, Us> const&>(rhs).leaf_get()),
				}...);
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
			tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>& lhs,
			tuple::IndexedTuple<meta::index_sequence<Is...>, Us...>& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us&>))) {
		VEG_ALL_OF(veg::swap( //
				static_cast<Ts&>(
						static_cast<tuple::TupleLeaf<Is, Ts>&>(lhs).leaf_get()),
				static_cast<Us&>(
						static_cast<tuple::TupleLeaf<Is, Us>&>(rhs).leaf_get())));
	}
};

} // namespace tup_
} // namespace internal
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
