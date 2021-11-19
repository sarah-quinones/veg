#ifndef VEG_INVOCABLE_HPP_GVSWRKAYS
#define VEG_INVOCABLE_HPP_GVSWRKAYS

#include "veg/type_traits/core.hpp"
#include "veg/type_traits/constructible.hpp"
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

namespace concepts {
VEG_CONCEPT_EXPR(
		(typename Fn, typename Ret, typename... Args),
		(Fn, Ret, Args...),
		fn_once,
		VEG_DECLVAL(Fn&&)(VEG_DECLVAL(Args&&)...),
		VEG_CONCEPT(same<ExprType, Ret>));

VEG_CONCEPT_EXPR(
		(typename Fn, typename Ret, typename... Args),
		(Fn, Ret, Args...),
		fn_mut,
		VEG_DECLVAL(Fn&)(VEG_DECLVAL(Args&&)...),
		VEG_CONCEPT(same<ExprType, Ret>));

VEG_CONCEPT_EXPR(
		(typename Fn, typename Ret, typename... Args),
		(Fn, Ret, Args...),
		fn,
		VEG_DECLVAL(Fn const&)(VEG_DECLVAL(Args&&)...),
		VEG_CONCEPT(same<ExprType, Ret>));
} // namespace concepts
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INVOCABLE_HPP_GVSWRKAYS */
