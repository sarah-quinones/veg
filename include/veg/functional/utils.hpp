#ifndef VEG_HELPERS_HPP_6TYDENKMS
#define VEG_HELPERS_HPP_6TYDENKMS

#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename... Fns>
struct Overload;

#if __cplusplus >= 201703

template <typename... Fns>
struct Overload : Fns... {
	using Fns::operator()...;
	template <typename... FFns>
	constexpr Overload(FFns... ffns)
			VEG_NOEXCEPT_LIKE(VEG_EVAL_ALL(VEG_FWD(ffns)()))
			: Fns{VEG_FWD(ffns)()}... {}
};

#else

template <>
struct Overload<> {
	void operator()() = delete;
};

#define VEG_TYPE_DECL(_, I)                                                    \
	__VEG_PP_COMMA_IF_NOT_0(I) typename __VEG_PP_CAT(Fn, I)
#define VEG_TYPE_PUT(_, I) __VEG_PP_COMMA_IF_NOT_0(I) __VEG_PP_CAT(Fn, I)
#define VEG_TYPE_INHERIT(_, I) __VEG_PP_COMMA_IF_NOT_0(I) __VEG_PP_CAT(Fn, I)

#define VEG_OVERLOAD_CALL(_, I) using __VEG_PP_CAT(Fn, I)::operator();
#define VEG_OVERLOAD_CTOR_DECL_TPL(_, I)                                       \
	__VEG_PP_COMMA_IF_NOT_0(I) typename __VEG_PP_CAT(FFn, I)
#define VEG_OVERLOAD_CTOR_DECL_FN(_, I)                                        \
	__VEG_PP_COMMA_IF_NOT_0(I) __VEG_PP_CAT(FFn, I) __VEG_PP_CAT(ffn, I)
#define VEG_OVERLOAD_CTOR_NOEXCEPT(_, I)                                       \
	__VEG_PP_COMMA_IF_NOT_0(I)(void) VEG_FWD(__VEG_PP_CAT(ffn, I))()
#define VEG_OVERLOAD_CTOR_INIT(_, I)                                           \
	__VEG_PP_COMMA_IF_NOT_0(I)                                                   \
	__VEG_PP_CAT(Fn, I) { VEG_FWD(__VEG_PP_CAT(ffn, I))() }

#define VEG_OVERLOAD_IMPL(Tuple)                                               \
	template <__VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, Tuple)>                  \
	struct Overload<__VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, Tuple)>             \
			: __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_INHERIT, _, Tuple) {                  \
		__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CALL, _, Tuple)                       \
		template <__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_DECL_TPL, _, Tuple)>   \
		VEG_INLINE constexpr Overload(                                             \
				__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_DECL_FN, _, Tuple))          \
				VEG_NOEXCEPT_LIKE(                                                     \
						(__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_NOEXCEPT, _, Tuple)))   \
				: __VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_INIT, _, Tuple) {}         \
	}

#define VEG_OVERLOAD_GENERIC(Tuple)                                            \
	template <__VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, Tuple), typename... Fns> \
	struct Overload<__VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, Tuple), Fns...>     \
			: __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_INHERIT, _, Tuple),                   \
				Overload<Fns...> {                                                     \
		__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CALL, _, Tuple)                       \
		using Overload<Fns...>::operator();                                        \
		template <                                                                 \
				__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_DECL_TPL, _, Tuple),         \
				typename... FFns>                                                      \
		VEG_INLINE constexpr Overload(                                             \
				__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_DECL_FN, _, Tuple),          \
				FFns... ffns)                                                          \
				VEG_NOEXCEPT_LIKE(                                                     \
						(__VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_NOEXCEPT, _, Tuple),    \
		         VEG_EVAL_ALL(VEG_FWD(ffns)())))                                   \
				: __VEG_PP_TUPLE_FOR_EACH(VEG_OVERLOAD_CTOR_INIT, _, Tuple),           \
					Overload<Fns...>{VEG_FWD(ffns)...} {}                                \
	}

VEG_OVERLOAD_IMPL((0));
VEG_OVERLOAD_IMPL((0, 1));
VEG_OVERLOAD_IMPL((0, 1, 2));
VEG_OVERLOAD_IMPL((0, 1, 2, 3));
VEG_OVERLOAD_IMPL((0, 1, 2, 3, 4));
VEG_OVERLOAD_IMPL((0, 1, 2, 3, 4, 5));
VEG_OVERLOAD_IMPL((0, 1, 2, 3, 4, 5, 6));
VEG_OVERLOAD_GENERIC((0, 1, 2, 3, 4, 5, 6, 7));

#undef VEG_TYPE_DECL
#undef VEG_TYPE_PUT
#undef VEG_TYPE_INHERIT
#undef VEG_OVERLOAD_CALL
#undef VEG_OVERLOAD_CTOR_DECL_TPL
#undef VEG_OVERLOAD_CTOR_DECL_FN
#undef VEG_OVERLOAD_CTOR_NOEXCEPT
#undef VEG_OVERLOAD_CTOR_INIT
#undef VEG_OVERLOAD_IMPL
#endif

template <i64 I, typename Fn>
struct IndexedFn {
	Fn fn;

	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_CONCEPT(invocable<Fn, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(/*tag*/, Fix<I>),
			(... args, Ts&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Ts...>))
					-> meta::invoke_result_t<Fn, Ts...> {
		return VEG_FWD(fn)(VEG_FWD(args)...);
	}
};

namespace nb {
template <typename To>
struct convert {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_CONCEPT(constructible<To, Ts...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Ts&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, Ts...>)) -> To {
		return To(VEG_FWD(args)...);
	}
};
struct clone {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(constructible<meta::decay_t<T>, T>)),
			VEG_INLINE constexpr auto
			operator(),
			(arg, T&&))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<meta::decay_t<T>, T>))
			->meta::decay_t<T> {
		return meta::decay_t<T>(VEG_FWD(arg));
	}
};
struct ref {
	template <typename T>
	VEG_INLINE constexpr auto operator()(T&& arg) const VEG_NOEXCEPT -> T&& {
		return VEG_FWD(arg);
	}
};
struct overload {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			VEG_INLINE constexpr auto
			operator(),
			(... fns, Fns))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->Overload<Fns...> {
		return {internal::ConvertingFn<Fns, Fns&&>{VEG_FWD(fns)}...};
	}
};
template <i64 I>
struct indexed {
	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(move_constructible<Fn>)),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->IndexedFn<I, Fn> {
		return {VEG_FWD(fn)};
	}
};
} // namespace nb
VEG_NIEBLOID(clone);
VEG_NIEBLOID(overload);
VEG_NIEBLOID(ref);
VEG_NIEBLOID_TEMPLATE(i64 I, indexed, I);
VEG_NIEBLOID_TEMPLATE(typename To, convert, To);
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_HELPERS_HPP_6TYDENKMS */
