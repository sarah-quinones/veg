#ifndef VEG_INVOCABLE_HPP_GVSWRKAYS
#define VEG_INVOCABLE_HPP_GVSWRKAYS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace meta_ {
template <typename Fn, typename... Args>
using call_expr = decltype(VEG_DECLVAL(Fn &&)(VEG_DECLVAL(Args &&)...));
} // namespace meta_
} // namespace internal
namespace meta {
template <typename Fn, typename... Args>
using invoke_result_t =
		meta::detected_t<internal::meta_::call_expr, Fn&&, Args&&...>;
} // namespace meta

inline namespace VEG_ABI {
namespace concepts {

namespace aux {
VEG_DEF_CONCEPT_DISJUNCTION(
		(typename T, typename U),
		matches_if_not_void,
		((concepts::, same<void, U>), (concepts::, same<T, U>)));

VEG_DEF_CONCEPT(
		(typename Fn, typename... Args),
		nothrow_invocable_pre,
		noexcept(VEG_DECLVAL_NOEXCEPT(Fn&&)(VEG_DECLVAL_NOEXCEPT(Args&&)...)));
} // namespace aux

VEG_DEF_CONCEPT(
		(typename Fn, typename... Args),
		invocable,
		VEG_CONCEPT(detected<internal::meta_::call_expr, Fn, Args&&...>));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename Fn, typename... Args),
		nothrow_invocable,
		((, invocable<Fn, Args&&...>),
     (aux::, nothrow_invocable_pre<Fn, Args&&...>)));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename Fn, typename Ret, typename... Args),
		invocable_r,
		((, invocable<Fn, Args&&...>),
     (aux::, matches_if_not_void<meta::invoke_result_t<Fn, Args&&...>, Ret>)));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename Fn, typename Ret, typename... Args),
		nothrow_invocable_r,
		((, invocable_r<Fn, Ret, Args&&...>),
     (, nothrow_invocable<Fn, Args&&...>)));

} // namespace concepts
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INVOCABLE_HPP_GVSWRKAYS */
