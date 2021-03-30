#ifndef VEG_MACROS_HPP_HSTLSKZXS
#define VEG_MACROS_HPP_HSTLSKZXS
#include "veg/internal/.external/hedley.ext.h"
#include <initializer_list>
#include "veg/internal/typedefs.hpp"
#include "veg/internal/preprocessor.hpp"
#include "veg/internal/prologue.hpp"

#if __cplusplus >= 201703L
#define VEG_ALL_OF(...) (__VA_ARGS__ && ... && true)
#define VEG_ANY_OF(...) (__VA_ARGS__ || ... || false)
#else
#define VEG_ALL_OF(...) ::veg::meta::all_of({(__VA_ARGS__)...})
#define VEG_ANY_OF(...) ::veg::meta::any_of({(__VA_ARGS__)...})
#endif

////////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 202002L

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
			::Namespace::__VA_ARGS__, __VEG_PP_STRINGIZE(__VA_ARGS__) " failed")
#define VEG_CONCEPT_MACRO(Namespace, ...) ::Namespace::__VA_ARGS__

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
	VEG_IGNORE_CPP14_EXTENSION_WARNING(                                          \
			template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                   \
			constexpr bool const& Name = ::veg::meta::bool_constant<Value>::value);  \
	}                                                                            \
	VEG_NOM_SEMICOLON
#else
#define __VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                          \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {}
#endif

#if __cplusplus >= 201402L
#define VEG_CONCEPT_MACRO(Namespace, ...) ::Namespace::__VA_ARGS__
#define __VEG_IMPL_ADD_VALUE(I, _, Param) (Param)
#define __VEG_IMPL_TRAIT(Param) __VEG_PP_HEAD Param _::__VEG_PP_TAIL Param
#else
#define VEG_CONCEPT_MACRO(Namespace, ...) ::Namespace::__VA_ARGS__::value
#define __VEG_IMPL_ADD_VALUE(I, _, Param)                                      \
	((__VEG_PP_REMOVE_PAREN(Param)::value))
#define __VEG_IMPL_TRAIT(Param) __VEG_PP_UNWRAP(Param)
#endif
#define __VEG_IMPL_PUT_TRAIT(I, _, Param) __VEG_IMPL_TRAIT(Param)

#define VEG_CHECK_CONCEPT_MACRO(Namespace, ...)                                \
	static_assert(                                                               \
			decltype(::Namespace::check_##__VA_ARGS__())::value,                     \
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
	void check_##Name() noexcept
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
	conjunction<__VEG_PP_REMOVE_PAREN(                                           \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_PUT_TRAIT, _, Tuple))>
#define __VEG_IMPL_DISJUNCTION(Tuple)                                          \
	disjunction<__VEG_PP_REMOVE_PAREN(                                           \
			__VEG_PP_TUPLE_TRANSFORM_I(__VEG_IMPL_PUT_TRAIT, _, Tuple))>

#endif

////////////////////////////////////////////////////////////////////////////////

#define VEG_TEMPLATE(TParams, Constraint, Attr_Name, ...)                      \
	__VEG_IMPL_TEMPLATE(                                                         \
			Attr_Name,                                                               \
			TParams,                                                                 \
			__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint),                           \
			__VA_ARGS__)

#if __cplusplus >= 202002L
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
			(bool __VEG_PP_CAT2(Dummy, __LINE__) = true),                            \
			requires(                                                                \
					__VEG_PP_CAT2(__VEG_IMPL_PREFIX_, Constraint) &&                     \
					::veg::meta::bool_constant<__VEG_PP_CAT2(Dummy, __LINE__)>::value),  \
			Attr_Name,                                                               \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__
#endif

#define __VEG_IMPL_PREFIX_requires
#define __VEG_IMPL_PREFIX_explicit

#define __VEG_IMPL_PARAM_EXPAND(I, _, Param)                                   \
	__VEG_PP_TAIL Param __VEG_PP_HEAD Param
#if __cplusplus >= 202002L
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

#define VEG_INLINE_VAR(name, obj)                                              \
	namespace /* NOLINT */ {                                                     \
	constexpr auto const& name =                                                 \
			::veg::internal::meta_::static_const<obj>::value;                        \
	}                                                                            \
	static_assert((void(name), true), ".")

#define VEG_NIEBLOID(name)                                   /* NOLINT */      \
	namespace /* NOLINT */ {                                   /* NOLINT */      \
	constexpr auto const& name =                               /* NOLINT */      \
			::veg::internal::meta_::static_const<nb::name>::value; /* NOLINT */      \
	}                                                          /* NOLINT */      \
	static_assert(((void)(name), true), ".")

#define VEG_FWD(x) static_cast<decltype(x)&&>(x)

// disallows moving const rvalues
#define VEG_MOV(x) static_cast<typename ::veg::uncvref_t<decltype(x)>&&>(x)

#ifdef VEG_NO_INSTANTIATE
#define VEG_INSTANTIATE(fn, ...) VEG_NOM_SEMICOLON
#define VEG_INSTANTIATE_CLASS(class_name, ...) VEG_NOM_SEMICOLON
#else
#define VEG_INSTANTIATE(fn, ...)                                               \
	__VEG_IMPL_INSTANTIATE(                                                      \
			fn,                                                                      \
			__VEG_PP_CAT(                                                            \
					__VEG_PP_CAT(_dummy_explicit_instantiation, __LINE__),               \
					__VEG_PP_CAT(_, __COUNTER__)),                                       \
			__VA_ARGS__)
#define __VEG_IMPL_INSTANTIATE(fn, name, ...)                                  \
	template <typename... Ts>                                                    \
	struct name {                                                                \
		void apply(Ts&&... args) { fn(VEG_FWD(args)...); }                         \
	};                                                                           \
	template struct name<__VA_ARGS__>

#define VEG_INSTANTIATE_CLASS(class_name, ...)                                 \
	template struct class_name<__VA_ARGS__>
#endif

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
struct static_const {
	static constexpr T value{};
};

template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)

template <typename T>
auto declval() noexcept -> T;
} // namespace meta_
} // namespace internal

template <typename... Ts>
struct incomplete_t;
template <typename... Types, typename... Args>
auto show_types(Args&&...) -> incomplete_t<Types..., Args...>;

namespace meta {
template <bool B, typename T = void>
using enable_if_t = typename internal::meta_::enable_if<B, T>::type;

template <typename T>
using uncvref_t = typename internal::meta_::uncvlref<T&>::type;

constexpr auto all_of_slice(bool const* arr, i64 size) noexcept -> bool {
	return size == 0 ? true : (arr[0] && meta::all_of_slice(arr + 1, size - 1));
}
constexpr auto all_of(std::initializer_list<bool> lst) noexcept -> bool {
	return meta::all_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}

constexpr auto any_of_slice(bool const* arr, i64 size) noexcept -> bool {
	return size == 0 ? false : (arr[0] || meta::any_of_slice(arr + 1, size - 1));
}
constexpr auto any_of(std::initializer_list<bool> lst) noexcept -> bool {
	return meta::any_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}
} // namespace meta
using meta::uncvref_t;
} // namespace VEG_ABI
} // namespace veg

#define VEG_CHECK_CONCEPT(...)                                                 \
	VEG_CHECK_CONCEPT_MACRO(veg::concepts, __VA_ARGS__)
#define VEG_CONCEPT(...) VEG_CONCEPT_MACRO(veg::concepts, __VA_ARGS__)

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_MACROS_HPP_HSTLSKZXS */
