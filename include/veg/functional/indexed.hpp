#ifndef VEG_INDEXED_HPP_4IARUYRPS
#define VEG_INDEXED_HPP_4IARUYRPS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace fn {

template <i64 I, typename Fn>
struct IndexedFn {
	Fn fn;

	VEG_TEMPLATE(
			(typename... Ts, typename Ret = meta::invoke_result_t<Fn const&, Ts...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(/*tag*/, Fix<I>),
			(... args, Ts))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn<Fn, Ret, Ts...>))->Ret {
		return fn(VEG_FWD(args)...);
	}
};

template <i64 I, typename Fn>
struct IndexedFnMut {
	Fn fn;

	VEG_TEMPLATE(
			(typename... Ts, typename Ret = meta::invoke_result_t<Fn&, Ts...>),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(/*tag*/, Fix<I>),
			(... args, Ts))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, Ts...>))->Ret {
		return fn(VEG_FWD(args)...);
	}
};

template <i64 I, typename Fn>
struct IndexedFnOnce {
	Fn fn;

	VEG_TEMPLATE(
			(typename... Ts, typename Ret = meta::invoke_result_t<Fn, Ts...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(/*tag*/, Fix<I>),
			(... args, Ts)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Ts...>)) -> Ret {
		return VEG_FWD(fn)(VEG_FWD(args)...);
	}
};

namespace nb {
template <i64 I>
struct indexed {
	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(move_constructible<Fn>)),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->IndexedFn<I, Fn> {
		return {Fn(VEG_FWD(fn))};
	}
};
template <i64 I>
struct indexed_mut {
	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(move_constructible<Fn>)),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->IndexedFnMut<I, Fn> {
		return {Fn(VEG_FWD(fn))};
	}
};
template <i64 I>
struct indexed_once {
	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(move_constructible<Fn>)),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->IndexedFnOnce<I, Fn> {
		return {Fn(VEG_FWD(fn))};
	}
};
} // namespace nb
VEG_NIEBLOID_TEMPLATE(i64 I, indexed, I);
VEG_NIEBLOID_TEMPLATE(i64 I, indexed_mut, I);
VEG_NIEBLOID_TEMPLATE(i64 I, indexed_once, I);
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INDEXED_HPP_4IARUYRPS */
