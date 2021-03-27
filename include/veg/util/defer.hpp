#ifndef VEG_DEFER_HPP_SQPONLRGS
#define VEG_DEFER_HPP_SQPONLRGS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
template <typename Fn>
struct VEG_NODISCARD defer {
	Fn fn;
	constexpr defer /* NOLINT */ (Fn _fn) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<Fn>))
			: fn(VEG_FWD(_fn)) {}
	defer(defer const&) = delete;
	defer(defer&&) noexcept = delete;
	auto operator=(defer const&) -> defer& = delete;
	auto operator=(defer&&) noexcept -> defer& = delete;
	VEG_CPP20(constexpr)
	HEDLEY_ALWAYS_INLINE ~defer() noexcept(
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn>))) {
		VEG_FWD(fn)();
	}
};
VEG_CPP17(

		template <typename Fn> defer(Fn) -> defer<Fn>;

)

namespace make {
namespace nb {
struct defer {
	VEG_TEMPLATE(
			typename Fn,
			requires(
					VEG_CONCEPT(move_constructible<Fn>) && VEG_CONCEPT(invocable<Fn>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr),
			auto
			operator(),
			(fn, Fn))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))->veg::defer<Fn> {
		return {VEG_FWD(fn)};
	}
};
} // namespace nb
VEG_NIEBLOID(defer);
} // namespace make
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DEFER_HPP_SQPONLRGS */
