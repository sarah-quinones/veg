#ifndef VEG_COPY_FN_HPP_FY5B8PSTS
#define VEG_COPY_FN_HPP_FY5B8PSTS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/functional/utils.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace fn {
template <typename Fn>
struct CopyFn {
	VEG_CHECK_CONCEPT(copy_constructible<Fn>);
	VEG_CHECK_CONCEPT(same<Fn, meta::decay_t<Fn>>);

	Fn fn;
	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(invocable<Fn, Args&&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const noexcept(
			VEG_CONCEPT(nothrow_copy_constructible<Fn>) &&
			VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>))
			->meta::invoke_result_t<Fn, Args&&...> {
		return (clone)(fn)(VEG_FWD(args)...);
	}
};
namespace nb {
struct copy_fn {
	VEG_TEMPLATE(
			typename Fn,
			requires(
					VEG_CONCEPT(constructible<meta::decay_t<Fn>, Fn&&>) &&
					VEG_CONCEPT(copy_constructible<meta::decay_t<Fn>>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->CopyFn<meta::decay_t<Fn>> {
		return {meta::decay_t<Fn>(VEG_FWD(fn))};
	}
};
} // namespace nb
VEG_NIEBLOID(copy_fn);
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_COPY_FN_HPP_FY5B8PSTS */
