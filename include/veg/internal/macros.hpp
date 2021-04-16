#ifndef VEG_MACROS_HPP_HSTLSKZXS
#define VEG_MACROS_HPP_HSTLSKZXS
#include "veg/internal/.external/hedley.ext.h"
#include <initializer_list>
#include "veg/internal/typedefs.hpp"
#include "veg/internal/preprocessor.hpp"
#include "veg/internal/prologue.hpp"

////////////////////////////////////////////////////////////////////////////////

#if HEDLEY_MSVC_VERSION_CHECK(14, 0, 0) ||                                     \
		HEDLEY_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define VEG_DEPRECATED(Reason) __declspec(deprecated(Reason))
#elif HEDLEY_HAS_EXTENSION(attribute_deprecated_with_message) ||               \
		HEDLEY_GCC_VERSION_CHECK(4, 5, 0) ||                                       \
		HEDLEY_INTEL_VERSION_CHECK(13, 0, 0) ||                                    \
		HEDLEY_ARM_VERSION_CHECK(5, 6, 0) ||                                       \
		HEDLEY_SUNPRO_VERSION_CHECK(5, 13, 0) ||                                   \
		HEDLEY_PGI_VERSION_CHECK(17, 10, 0) ||                                     \
		HEDLEY_TI_VERSION_CHECK(18, 1, 0) ||                                       \
		HEDLEY_TI_ARMCL_VERSION_CHECK(18, 1, 0) ||                                 \
		HEDLEY_TI_CL6X_VERSION_CHECK(8, 3, 0) ||                                   \
		HEDLEY_TI_CL7X_VERSION_CHECK(1, 2, 0) ||                                   \
		HEDLEY_TI_CLPRU_VERSION_CHECK(2, 3, 0)
#define VEG_DEPRECATED(Reason) __attribute__((__deprecated__(Reason)))
#elif defined(__cplusplus) && (__cplusplus >= 201402L)
#define VEG_DEPRECATED(Reason)                                                 \
	HEDLEY_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[deprecated(Reason)]])
#elif HEDLEY_HAS_ATTRIBUTE(deprecated) || HEDLEY_GCC_VERSION_CHECK(3, 1, 0) || \
		HEDLEY_ARM_VERSION_CHECK(4, 1, 0) || HEDLEY_TI_VERSION_CHECK(15, 12, 0) || \
		(HEDLEY_TI_ARMCL_VERSION_CHECK(4, 8, 0) &&                                 \
     defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) ||                                 \
		HEDLEY_TI_ARMCL_VERSION_CHECK(5, 2, 0) ||                                  \
		(HEDLEY_TI_CL2000_VERSION_CHECK(6, 0, 0) &&                                \
     defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) ||                                 \
		HEDLEY_TI_CL2000_VERSION_CHECK(6, 4, 0) ||                                 \
		(HEDLEY_TI_CL430_VERSION_CHECK(4, 0, 0) &&                                 \
     defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) ||                                 \
		HEDLEY_TI_CL430_VERSION_CHECK(4, 3, 0) ||                                  \
		(HEDLEY_TI_CL6X_VERSION_CHECK(7, 2, 0) &&                                  \
     defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) ||                                 \
		HEDLEY_TI_CL6X_VERSION_CHECK(7, 5, 0) ||                                   \
		HEDLEY_TI_CL7X_VERSION_CHECK(1, 2, 0) ||                                   \
		HEDLEY_TI_CLPRU_VERSION_CHECK(2, 1, 0)
#define VEG_DEPRECATED(Reason) __attribute__((__deprecated__))
#elif HEDLEY_MSVC_VERSION_CHECK(13, 10, 0) ||                                  \
		HEDLEY_PELLES_VERSION_CHECK(6, 50, 0) ||                                   \
		HEDLEY_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define VEG_DEPRECATED(Reason) __declspec(deprecated)
#elif HEDLEY_IAR_VERSION_CHECK(8, 0, 0)
#define VEG_DEPRECATED(Reason) _Pragma("deprecated")
#else
#define VEG_DEPRECATED(Reason)
#endif

////////////////////////////////////////////////////////////////////////////////

#define VEG_DECLVAL(...) (static_cast<__VA_ARGS__ (*)()>(nullptr)())
#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#define VEG_HAS_CONCEPTS 1
#else
#endif

#if __cplusplus >= 201703L
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
	(static_cast<__VA_ARGS__ (*)() noexcept>(nullptr)())
#else
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
	(::veg::internal::meta_::declval<__VA_ARGS__>())
#endif

#define VEG_ARROW(...)                                                         \
	noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { return __VA_ARGS__; }

#define VEG_LIFT(...)                                                          \
	[&](auto&&... args) VEG_ARROW((__VA_ARGS__)(VEG_FWD(args)...))

#define VEG_DEDUCE_RET(...) VEG_ARROW(__VA_ARGS__) VEG_NOM_SEMICOLON

#if __cplusplus >= 201703L
#define VEG_ALL_OF(...) (__VA_ARGS__ && ... && true)
#define VEG_ANY_OF(...) (__VA_ARGS__ || ... || false)
#else
#define VEG_ALL_OF(...)                                                        \
	::veg::meta::and_test<                                                       \
			::veg::meta::make_index_sequence<                                        \
					::veg::meta::pack_size<decltype((void)(__VA_ARGS__))...>::value>,    \
			::veg::meta::bool_constant<(__VA_ARGS__)>...>::value
#define VEG_ANY_OF(...)                                                        \
	::veg::meta::or_test<                                                        \
			::veg::meta::make_index_sequence<                                        \
					::veg::meta::pack_size<decltype((void)(__VA_ARGS__))...>::value>,    \
			::veg::meta::bool_constant<(__VA_ARGS__)>...>::value
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef VEG_HAS_CONCEPTS

#define __VEG_IMPL_AND(_, Param) &&__VEG_PP_UNWRAP(Param)
#define __VEG_IMPL_OR(_, Param) || __VEG_PP_UNWRAP(Param)

#define __VEG_IMPL_CONJUNCTION(First, ...)                                     \
	(__VEG_PP_UNWRAP(First)                                                      \
	     __VEG_PP_TUPLE_FOR_EACH(__VEG_IMPL_AND, _, (__VA_ARGS__)))

#define __VEG_IMPL_DISJUNCTION(First, ...)                                     \
	(__VEG_PP_UNWRAP(First)                                                      \
	     __VEG_PP_TUPLE_FOR_EACH(__VEG_IMPL_OR, _, (__VA_ARGS__)))

#define VEG_DEF_CONCEPT(Tpl, Name, ...)                                        \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	concept Name = __VA_ARGS__;

#define VEG_CHECK_CONCEPT_MACRO(Namespace, ...)                                \
	static_assert(                                                               \
			Namespace::__VA_ARGS__, __VEG_PP_STRINGIZE(__VA_ARGS__) " failed")
#define VEG_CONCEPT_MACRO(Namespace, ...) Namespace::__VA_ARGS__

#define VEG_DEF_CONCEPT_CONJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT(Tpl, Name, __VEG_IMPL_CONJUNCTION Terms)
#define VEG_DEF_CONCEPT_DISJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT(Tpl, Name, __VEG_IMPL_DISJUNCTION Terms)

#else

#if __cplusplus >= 201703L
#define __VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                          \
	namespace _ {                                                                \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {};                                                \
	}                                                                            \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	inline constexpr bool const& Name = Value
#elif __cplusplus >= 201402L
#define __VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                          \
	namespace _ {                                                                \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {};                                                \
	}                                                                            \
	namespace {                                                                  \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	constexpr bool const& Name = ::veg::meta::bool_constant<Value>::value;       \
	}                                                                            \
	VEG_NOM_SEMICOLON
#else
#define __VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                          \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {}
#endif

#if __cplusplus >= 201402L
#define VEG_CONCEPT_MACRO(Namespace, ...) Namespace::__VA_ARGS__
#define __VEG_IMPL_ADD_VALUE(I, _, Param) (Param)
#define __VEG_IMPL_TRAIT(Param) __VEG_PP_HEAD Param _::__VEG_PP_TAIL Param
#else
#define VEG_CONCEPT_MACRO(Namespace, ...) Namespace::__VA_ARGS__::value
#define __VEG_IMPL_ADD_VALUE(I, _, Param)                                      \
	((__VEG_PP_REMOVE_PAREN(Param)::value))
#define __VEG_IMPL_TRAIT(Param) __VEG_PP_UNWRAP(Param)
#endif
#define __VEG_IMPL_PUT_TRAIT(I, _, Param) __VEG_IMPL_TRAIT(Param)

#define VEG_CHECK_CONCEPT_MACRO(Namespace, ...)                                \
	static_assert(                                                               \
			decltype(Namespace::check_##__VA_ARGS__())::value,                       \
			__VEG_PP_STRINGIZE(__VA_ARGS__) " failed")
#define VEG_DEF_CONCEPT(Tpl, Name, ...)                                        \
	__VEG_IMPL_DEF_CONCEPT(                                                      \
			Tpl, Name, (__VA_ARGS__), ::veg::meta::bool_constant<__VA_ARGS__>);      \
	VEG_TEMPLATE(                                                                \
			Tpl, requires(__VA_ARGS__), constexpr auto check_##Name, (_ = 0, int))   \
	noexcept->::veg::meta::true_type

#define __VEG_IMPL_SFINAE(_, Param)                                            \
	, ::veg::meta::enable_if_t<__VEG_PP_UNWRAP Param, int> = 0

#define __VEG_IMPL_OVERLOAD(Name_Tpl, Param)                                   \
	template <                                                                   \
			__VEG_PP_REMOVE_PAREN(__VEG_PP_TAIL Name_Tpl),                           \
			::veg::meta::enable_if_t<__VEG_PP_UNWRAP Param, int> = 0>                \
	void __VEG_PP_CAT(check_, __VEG_PP_HEAD Name_Tpl)() noexcept;

#define VEG_DEF_CONCEPT_BOOL_CONJUNCTION_IMPL(Tpl, Name, Base, Seq)            \
	__VEG_IMPL_DEF_CONCEPT(                                                      \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_PP_REMOVE_PAREN1(Base)::value),                                   \
			__VEG_PP_REMOVE_PAREN1(Base));                                           \
	template <__VEG_PP_REMOVE_PAREN(Tpl)                                         \
	              __VEG_PP_TUPLE_FOR_EACH(__VEG_IMPL_SFINAE, _, Seq)>            \
	auto check_##Name() noexcept->::veg::meta::true_type
#define VEG_DEF_CONCEPT_BOOL_DISJUNCTION_IMPL(Tpl, Name, Base, Seq)            \
	__VEG_IMPL_DEF_CONCEPT(                                                      \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_PP_REMOVE_PAREN1(Base)::value),                                   \
			__VEG_PP_REMOVE_PAREN1(Base));                                           \
	__VEG_PP_TUPLE_FOR_EACH(__VEG_IMPL_OVERLOAD, (Name, Tpl), Seq)               \
	VEG_NOM_SEMICOLON

#define VEG_DEF_CONCEPT_CONJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT_BOOL_CONJUNCTION_IMPL(                                       \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_IMPL_CONJUNCTION(Terms)),                                         \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_ADD_VALUE, _, Terms))

#define VEG_DEF_CONCEPT_DISJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT_BOOL_DISJUNCTION_IMPL(                                       \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_IMPL_DISJUNCTION(Terms)),                                         \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_ADD_VALUE, _, Terms))

#define __VEG_IMPL_CONJUNCTION(Tuple)                                          \
	::veg::meta::conjunction<__VEG_PP_REMOVE_PAREN(                              \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_PUT_TRAIT, _, Tuple))>
#define __VEG_IMPL_DISJUNCTION(Tuple)                                          \
	::veg::meta::disjunction<__VEG_PP_REMOVE_PAREN(                              \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_PUT_TRAIT, _, Tuple))>

#endif

////////////////////////////////////////////////////////////////////////////////

#define VEG_TEMPLATE(TParams, Constraint, Attr_Name, ...)                      \
	__VEG_IMPL_TEMPLATE(                                                         \
			Attr_Name,                                                               \
			TParams,                                                                 \
			__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint),                           \
			__VA_ARGS__)

#ifdef VEG_HAS_CONCEPTS
#define VEG_TEMPLATE_EXPLICIT(                                                 \
		TParams, Constraint, Explicit_Cond, Attr_Name, Params, ...)                \
	VEG_TEMPLATE(                                                                \
			TParams,                                                                 \
			Constraint,                                                              \
			Explicit_Cond Attr_Name,                                                 \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__

#define VEG_CONSTRAINED_MEMBER_FN(Constraint, Attr_Name, Params, ...)          \
	Attr_Name __VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_PARAM_EXPAND, _, Params)     \
			__VA_ARGS__ requires __VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint)

#define VEG_TEMPLATE_CVT(TParams, Constraint, Attr, ...)                       \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	Constraint Attr operator __VA_ARGS__()
#else
#define VEG_TEMPLATE_EXPLICIT(                                                 \
		TParams, Constraint, Explicit_Cond, Attr_Name, Params, ...)                \
	VEG_TEMPLATE(                                                                \
			(__VEG_PP_REMOVE_PAREN TParams,                                          \
	     ::veg::meta::enable_if_t<                                               \
					 (__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Explicit_Cond)),                 \
					 int> = 0),                                                          \
			Constraint,                                                              \
			explicit Attr_Name,                                                      \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__                                                                  \
	VEG_TEMPLATE(                                                                \
			(__VEG_PP_REMOVE_PAREN TParams,                                          \
	     ::veg::meta::enable_if_t<                                               \
					 !(__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Explicit_Cond)),                \
					 unsigned> = 0),                                                     \
			Constraint,                                                              \
			Attr_Name,                                                               \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__

#define VEG_CONSTRAINED_MEMBER_FN(Constraint, Attr_Name, Params, ...)          \
	VEG_TEMPLATE(                                                                \
			(int _ = 0),                                                             \
			requires(                                                                \
					__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint) &&                     \
					::veg::meta::bool_constant<(_ == 0)>::value),                        \
			Attr_Name,                                                               \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__

#define VEG_TEMPLATE_CVT(TParams, Constraint, Attr, ...)                       \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	Attr operator ::veg::internal::meta_::discard_1st<                           \
			::veg::meta::enable_if_t<(                                               \
					__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint))>,                     \
			__VA_ARGS__>()
#endif

#define __VEG_IMPL_PREFIX_requires
#define __VEG_IMPL_PREFIX_explicit

#define __VEG_IMPL_PARAM_EXPAND(I, _, Param)                                   \
	__VEG_PP_TAIL Param __VEG_PP_HEAD Param
#ifdef VEG_HAS_CONCEPTS
#define __VEG_IMPL_TEMPLATE(Attr_Name, TParams, Constraint, ...)               \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	requires Constraint Attr_Name __VEG_PP_TUPLE_TRANSFORM_I(                    \
			__VEG_IMPL_PARAM_EXPAND, _, (__VA_ARGS__))
#else

#define __VEG_IMPL_TEMPLATE2_HELPER_0(Constraint, Param)                       \
	__VEG_PP_TAIL Param __VEG_PP_HEAD Param

#define __VEG_IMPL_TEMPLATE2_HELPER_1(Constraint, Param)                       \
	::veg::internal::meta_::                                                     \
			discard_1st<::veg::meta::enable_if_t<(Constraint)>, __VEG_PP_TAIL Param> \
					__VEG_PP_HEAD Param

#define __VEG_IMPL_TEMPLATE2_HELPER(I, Constraint, Param)                      \
	__VEG_PP_CAT2(                                                               \
			__VEG_IMPL_TEMPLATE2_HELPER_, __VEG_IMPL_PP_IS_1(__VEG_IMPL_PP_INC(I)))  \
	(Constraint, Param)

#define __VEG_IMPL_TEMPLATE(Attr_Name, TParams, Constraint, ...)               \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	Attr_Name __VEG_PP_TUPLE_TRANSFORM_I(                                        \
			__VEG_IMPL_TEMPLATE2_HELPER, Constraint, (__VA_ARGS__))

#endif

////////////////////////////////////////////////////////////////////////////////

#define VEG_INLINE_VAR(Name, Obj)                                              \
	namespace /* NOLINT */ {                                                     \
	constexpr auto const& Name = ::veg::meta::static_const<Obj>::value;          \
	}                                                                            \
	static_assert((void(Name), true), ".")

#if __cplusplus >= 201402L
#define VEG_INLINE_VAR_TEMPLATE(Tpl, Name, ...) /* NOLINT */                   \
	namespace /* NOLINT */ {                      /* NOLINT */                   \
	template <__VEG_PP_REMOVE_PAREN(Tpl)>                                        \
	constexpr auto const& Name =                       /* NOLINT */              \
			::veg::meta::static_const<__VA_ARGS__>::value; /* NOLINT */              \
	}                                                                            \
	VEG_NOM_SEMICOLON /* NOLINT */
#else
#define VEG_INLINE_VAR_TEMPLATE(Tpl, Name, ...) VEG_NOM_SEMICOLON
#endif

#define VEG_NIEBLOID(Name) VEG_INLINE_VAR(Name, nb::Name)

#define VEG_NIEBLOID_TEMPLATE(Tpl, Name, ...)                                  \
	VEG_INLINE_VAR_TEMPLATE(Tpl, Name, nb::Name<__VA_ARGS__>)

#define VEG_FWD(X) static_cast<decltype(X)&&>(X)

// disallows moving const rvalues
#define VEG_MOV(X) static_cast<typename ::veg::uncvref_t<decltype(X)>&&>(X)

#ifdef VEG_NO_INSTANTIATE
#define VEG_INSTANTIATE(Fn, ...) VEG_NOM_SEMICOLON
#define VEG_INSTANTIATE_CLASS(Class, ...) VEG_NOM_SEMICOLON
#else
#define VEG_INSTANTIATE(Fn, ...)                                               \
	__VEG_IMPL_INSTANTIATE(                                                      \
			Fn,                                                                      \
			__VEG_PP_CAT(                                                            \
					__VEG_PP_CAT(_dummy_explicit_instantiation, __LINE__),               \
					__VEG_PP_CAT(_, __COUNTER__)),                                       \
			__VA_ARGS__)
#define __VEG_IMPL_INSTANTIATE(Fn, Name, ...)                                  \
	template <typename... Ts>                                                    \
	struct Name {                                                                \
		void apply(Ts&&... args) { Fn(VEG_FWD(args)...); }                         \
	};                                                                           \
	template struct Name<__VA_ARGS__>

#define VEG_INSTANTIATE_CLASS(Class, ...) template struct Class<__VA_ARGS__>
#endif

#define VEG_NOM_SEMICOLON static_assert(true, ".")

namespace veg {
inline namespace VEG_ABI {
namespace internal {
namespace meta_ {

template <bool B, typename T = void>
struct enable_if {
	using type = T;
};
template <typename T>
struct enable_if<false, T> {};

template <typename U, typename V>
using discard_1st = V;

template <typename T>
struct uncvlref;
template <typename T>
struct uncvlref<T&> {
	using type = T;
};
template <typename T>
struct uncvlref<T const&> {
	using type = T;
};
template <typename T>
struct uncvlref<T volatile&> {
	using type = T;
};
template <typename T>
struct uncvlref<T volatile const&> {
	using type = T;
};

template <typename T>
struct unref;
template <typename T>
struct unref<T&> {
	using type = T;
};

template <typename T>
auto declval() noexcept -> T;
} // namespace meta_
} // namespace internal

namespace meta {
template <typename T>
struct static_const {
	static constexpr T value{};
};

template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)
} // namespace meta

template <typename... Ts>
struct incomplete_t;

template <typename... Types, typename... Args>
auto print_types_halt(Args&&...) -> incomplete_t<Types..., Args...>;
template <typename... Types, typename... Args>
VEG_DEPRECATED("")
void print_types(Args&&... /*unused*/) {}

namespace nb {
struct unused {
	template <typename... Ts>
	VEG_CPP14(constexpr)
	void operator()(Ts const&... /*unused*/) const noexcept {}
};
} // namespace nb
VEG_NIEBLOID(unused);

namespace internal {
constexpr auto all_of_slice(bool const* arr, i64 size) noexcept -> bool {
	return size == 0 ? true
									 : (arr[0] && internal::all_of_slice(arr + 1, size - 1));
}
constexpr auto all_of(std::initializer_list<bool> lst) noexcept -> bool {
	return internal::all_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}
} // namespace internal
namespace meta {
template <bool B, typename T = void>
using enable_if_t = typename internal::meta_::enable_if<B, T>::type;

template <typename T>
using uncvref_t = typename internal::meta_::uncvlref<T&>::type;
} // namespace meta
using meta::uncvref_t;
} // namespace VEG_ABI
} // namespace veg

#define VEG_CHECK_CONCEPT(...)                                                 \
	VEG_CHECK_CONCEPT_MACRO(::veg::concepts, __VA_ARGS__)
#define VEG_CONCEPT(...) VEG_CONCEPT_MACRO(::veg::concepts, __VA_ARGS__)

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_MACROS_HPP_HSTLSKZXS */
