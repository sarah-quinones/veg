#ifndef VEG_COMPOSE_HPP_XCUOP6MPS
#define VEG_COMPOSE_HPP_XCUOP6MPS

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

namespace internal {
template <typename T>
struct Wrapper {
	T inner;
};
} // namespace internal

namespace fn {
template <typename... Fns>
struct ComposeOnce;
template <typename... Fns>
struct ComposeMut;
template <typename... Fns>
struct Compose;

template <>
struct ComposeOnce<> {

	// a forwarding reference is used to allow composing with `ref, mut`
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(arg, T&&))
	&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T&& {
		return VEG_FWD(arg);
	}
};
template <>
struct ComposeMut<> {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(arg, T&&))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T&& {
		return VEG_FWD(arg);
	}
};
template <>
struct Compose<> {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_INLINE constexpr auto
			operator(),
			(arg, T&&))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T&& {
		return VEG_FWD(arg);
	}
};

template <typename First, typename... RestElems>
struct ComposeOnce<First, RestElems...> {
	using Rest = ComposeOnce<RestElems...>;
	internal::Wrapper<First> first;
	internal::Wrapper<Rest> rest;

	VEG_TEMPLATE(
			(typename... Args,
	     typename RetTail = meta::invoke_result_t<Rest, Args...>,
	     typename Ret = meta::invoke_result_t<First, RetTail>),
			requires(
					VEG_CONCEPT(fn_once<Rest, RetTail, Args...>) &&
					VEG_CONCEPT(fn_once<First, Ret, RetTail>)),
			VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_fn_once<Rest, RetTail, Args...>) &&
					VEG_CONCEPT(nothrow_fn_once<First, Ret, RetTail>)) -> Ret {
		return VEG_FWD(first.inner)(VEG_FWD(rest.inner)(VEG_FWD(args)...));
	}
};
template <typename First, typename... RestElems>
struct ComposeMut<First, RestElems...> {
	using Rest = ComposeMut<RestElems...>;
	internal::Wrapper<First> first;
	internal::Wrapper<Rest> rest;

	VEG_TEMPLATE(
			(typename... Args,
	     typename RetTail = meta::invoke_result_t<Rest&, Args...>,
	     typename Ret = meta::invoke_result_t<First&, RetTail>),
			requires(
					VEG_CONCEPT(fn_mut<Rest, RetTail, Args...>) &&
					VEG_CONCEPT(fn_mut<First, Ret, RetTail>)),
			VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_mut<Rest, RetTail, Args...>) &&
			VEG_CONCEPT(nothrow_fn_mut<First, Ret, RetTail>))
			->Ret {
		return first.inner(rest.inner(VEG_FWD(args)...));
	}
};
template <typename First, typename... RestElems>
struct Compose<First, RestElems...> {
	using Rest = Compose<RestElems...>;
	internal::Wrapper<First> first;
	internal::Wrapper<Rest> rest;

	VEG_TEMPLATE(
			(typename... Args,
	     typename RetTail = meta::invoke_result_t<Rest const&, Args...>,
	     typename Ret = meta::invoke_result_t<First const&, RetTail>),
			requires(
					VEG_CONCEPT(fn<Rest, RetTail, Args...>) &&
					VEG_CONCEPT(fn<First, Ret, RetTail>)),
			constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Rest, RetTail, Args...>) &&
			VEG_CONCEPT(nothrow_fn<First, Ret, RetTail>))
			->Ret {
		return first.inner(rest.inner(VEG_FWD(args)...));
	}
};

#ifdef __clang__
HEDLEY_DIAGNOSTIC_PUSH
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

namespace nb {
struct compose_once {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			constexpr auto
			operator(),
			(... fns, Fns))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->ComposeOnce<Fns...> {

		/* https://eel.is/c++draft/dcl.init.aggr#15
		 * https://eel.is/c++draft/dcl.init.aggr#16
		 * interal::Wrapper prevents implicit conversions of the inner type from
		 * being invoked instead of falling back to brace elision
		 * i hate c++
		 */

		return {
				internal::Wrapper<Fns>{Fns(VEG_FWD(fns))}...,
				{},
		};
	}
};
struct compose_mut {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			constexpr auto
			operator(),
			(... fns, Fns))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->ComposeMut<Fns...> {
		return {internal::Wrapper<Fns>{Fns(VEG_FWD(fns))}..., {}};
	}
}; // namespace nb
struct compose {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			constexpr auto
			operator(),
			(... fns, Fns))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->Compose<Fns...> {
		return {internal::Wrapper<Fns>{Fns(VEG_FWD(fns))}..., {}};
	}
}; // namespace nb
} // namespace nb

#ifdef __clang__
HEDLEY_DIAGNOSTIC_POP
#endif

VEG_NIEBLOID(compose_once);
VEG_NIEBLOID(compose_mut);
VEG_NIEBLOID(compose);
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_COMPOSE_HPP_XCUOP6MPS */
