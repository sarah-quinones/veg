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

namespace aux {
VEG_DEF_CONCEPT(
		(typename Fn, typename... Args),
		nothrow_invocable_pre,
		VEG_IS_NOEXCEPT(
				VEG_DECLVAL_NOEXCEPT(Fn&&)(VEG_DECLVAL_NOEXCEPT(Args&&)...)));
} // namespace aux

#if __cplusplus >= 202002L

template <typename Fn, typename... Args>
concept invocable =
		requires(Fn && (&make_fn)() noexcept, Args && (&... make_args)() noexcept) {
	make_fn()(make_args()...);
};
template <typename Fn, typename... Args>
concept nothrow_invocable =
		requires(Fn && (&make_fn)() noexcept, Args && (&... make_args)() noexcept) {
	{ make_fn()(make_args()...) }
	noexcept;
};

template <typename Fn, typename Ret, typename... Args>
concept invocable_r =
		requires(Fn && (&make_fn)() noexcept, Args && (&... make_args)() noexcept) {
	{ make_fn()(make_args()...) } -> same<Ret>;
};
template <typename Fn, typename Ret, typename... Args>
concept nothrow_invocable_r =
		requires(Fn && (&make_fn)() noexcept, Args && (&... make_args)() noexcept) {
	{ make_fn()(make_args()...) }
	noexcept->same<Ret>;
};

#else
VEG_DEF_CONCEPT(
		(typename Fn, typename... Args),
		invocable,
		VEG_CONCEPT(detected<internal::meta_::call_expr, Fn, Args&&...>));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename Fn, typename... Args),
		nothrow_invocable,
		((, invocable<Fn, Args&&...>),
     (aux::, nothrow_invocable_pre<Fn, Args&&...>)));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		invocable_r,
		(VEG_CONCEPT(invocable<Fn, Args&&...>) &&
     VEG_CONCEPT(same<Ret, meta::invoke_result_t<Fn, Args&&...>>)));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		nothrow_invocable_r,
		(VEG_CONCEPT(nothrow_invocable<Fn, Args...>) &&
     VEG_CONCEPT(invocable_r<Fn, Ret, Args...>)));

#endif

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		fn,
		VEG_CONCEPT(invocable_r<Fn const&, Ret, Args...>));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		nothrow_fn,
		VEG_CONCEPT(nothrow_invocable_r<Fn const&, Ret, Args...>));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		fn_mut,
		VEG_CONCEPT(invocable_r<Fn&, Ret, Args...>));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		nothrow_fn_mut,
		VEG_CONCEPT(nothrow_invocable_r<Fn&, Ret, Args...>));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		fn_once,
		VEG_CONCEPT(invocable_r<Fn, Ret, Args...>));

VEG_DEF_CONCEPT(
		(typename Fn, typename Ret, typename... Args),
		nothrow_fn_once,
		VEG_CONCEPT(nothrow_invocable_r<Fn, Ret, Args...>));

} // namespace concepts
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INVOCABLE_HPP_GVSWRKAYS */
