#ifndef VEG_COMPOSE_HPP_XCUOP6MPS
#define VEG_COMPOSE_HPP_XCUOP6MPS

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
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
			(typename Self, typename... Args),
			requires(
					VEG_CONCEPT(invocable<Compose<Rest...>, Args&&...>) &&
					VEG_CONCEPT(invocable<
											First,
											meta::invoke_result_t<Compose<Rest...>, Args&&...>>)),
			constexpr auto
			operator(),
			(... args,
	     Args&&)) && noexcept(noexcept(VEG_FWD(first)(rest(VEG_FWD(args)...))))
			-> meta::invoke_result_t<
					First,
					meta::invoke_result_t<Compose<Rest...>, Args&&...>> {
		return VEG_FWD(first)(rest(VEG_FWD(args)...));
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
		return {VEG_FWD(fns)...};
	}
};
} // namespace nb
VEG_NIEBLOID(compose_fwd);
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_COMPOSE_HPP_XCUOP6MPS */
