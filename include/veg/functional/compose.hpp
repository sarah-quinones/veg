#ifndef VEG_COMPOSE_HPP_XCUOP6MPS
#define VEG_COMPOSE_HPP_XCUOP6MPS

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/functional/utils.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace fn {
template <typename... Fns>
struct Compose;

template <>
struct Compose<> {
	template <typename T>
	constexpr auto operator()(T&& arg) noexcept -> T&& {
		return VEG_FWD(arg);
	}
};

template <typename First, typename... Rest>
struct Compose<First, Rest...> {
	First first;
	Compose<Rest...> rest;

	VEG_TEMPLATE(
			(typename... Args),
			requires(
					VEG_CONCEPT(invocable<Compose<Rest...>, Args&&...>) &&
					VEG_CONCEPT(invocable<
											First,
											meta::invoke_result_t<Compose<Rest...>, Args&&...>>)),
			constexpr auto
			operator(),
			(... args,
	     Args&&)) && noexcept(noexcept(VEG_FWD(first)(VEG_FWD(rest)(VEG_FWD(args)...))))
			-> meta::invoke_result_t<
					First,
					meta::invoke_result_t<Compose<Rest...>, Args&&...>> {
		return VEG_FWD(first)(VEG_FWD(rest)(VEG_FWD(args)...));
	}

	VEG_TEMPLATE(
			(typename... Args),
			requires(
					VEG_CONCEPT(copy_constructible<First>) &&
					VEG_ALL_OF(VEG_CONCEPT(copy_constructible<Rest>)) &&
					VEG_CONCEPT(invocable<Compose<Rest...>, Args&&...>) &&
					VEG_CONCEPT(invocable<
											First,
											meta::invoke_result_t<Compose<Rest...>, Args&&...>>)),
			constexpr auto
			operator(),
			(... args, Args&&))
	const& noexcept(noexcept(VEG_FWD(first)(VEG_FWD(rest)(VEG_FWD(args)...))))
			->meta::invoke_result_t<
					First,
					meta::invoke_result_t<Compose<Rest...>, Args&&...>> {
		return VEG_FWD((clone)(first))(VEG_FWD((clone)(rest))(VEG_FWD(args)...));
	}
};

namespace nb {
struct compose_fwd {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			constexpr auto
			operator(),
			(... fns, Fns&&))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->Compose<Fns...> {
#ifdef __clang__
		HEDLEY_DIAGNOSTIC_PUSH
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

		// https://eel.is/c++draft/dcl.init.aggr#15
		return {Fns(VEG_FWD(fns))..., {}};

#ifdef __clang__
		HEDLEY_DIAGNOSTIC_POP
#endif
	}
}; // namespace nb
} // namespace nb
VEG_NIEBLOID(compose_fwd);
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_COMPOSE_HPP_XCUOP6MPS */
