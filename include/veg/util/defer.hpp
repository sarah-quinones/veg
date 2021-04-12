#ifndef VEG_DEFER_HPP_SQPONLRGS
#define VEG_DEFER_HPP_SQPONLRGS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
template <typename Fn>
struct VEG_NODISCARD Defer {
	Fn fn;
	constexpr Defer(Fn _fn) noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			: fn(VEG_FWD(_fn)) {}
	Defer(Defer const&) = delete;
	Defer(Defer&&) noexcept = delete;
	auto operator=(Defer const&) -> Defer& = delete;
	auto operator=(Defer&&) noexcept -> Defer& = delete;
	VEG_CPP20(constexpr)
	HEDLEY_ALWAYS_INLINE ~Defer() noexcept(
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn>))) {
		VEG_FWD(fn)();
	}
};
VEG_CPP17(

		template <typename Fn> Defer(Fn) -> Defer<Fn>;

)

namespace nb {
struct defer {
	VEG_TEMPLATE(
			typename Fn,
			requires(
					VEG_CONCEPT(move_constructible<Fn>) && VEG_CONCEPT(invocable<Fn>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) auto
			operator(),
			(fn, Fn&&))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))->veg::Defer<Fn> {
		return {VEG_FWD(fn)};
	}
};
} // namespace nb
VEG_NIEBLOID(defer);
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DEFER_HPP_SQPONLRGS */
