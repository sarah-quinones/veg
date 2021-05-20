#ifndef VEG_COMPOSE_HPP_XCUOP6MPS
#define VEG_COMPOSE_HPP_XCUOP6MPS

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/functional/utils.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace fn {
template <typename... Fns>
struct Compose;

template <>
struct Compose<> {
	template <typename T>
	constexpr auto operator()(T&& arg) VEG_NOEXCEPT -> T&& {
		return VEG_FWD(arg);
	}
};

template <typename First, typename... RestElems>
struct Compose<First, RestElems...> {
	using Rest = Compose<RestElems...>;
	First first;
	Rest rest;

	VEG_TEMPLATE(
			(typename... Args),
			requires(
					VEG_CONCEPT(invocable<Rest, Args&&...>) &&
					VEG_CONCEPT(
							invocable<First, meta::invoke_result_t<Rest, Args&&...>>)),
			constexpr auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(VEG_IS_NOEXCEPT(
					VEG_FWD(first)(VEG_FWD(rest)(VEG_FWD(args)...)))) -> meta::
					invoke_result_t<First, meta::invoke_result_t<Rest, Args&&...>> {
		return VEG_FWD(first)(VEG_FWD(rest)(VEG_FWD(args)...));
	}
};

namespace nb {
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
VEG_NIEBLOID(compose);
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_COMPOSE_HPP_XCUOP6MPS */
