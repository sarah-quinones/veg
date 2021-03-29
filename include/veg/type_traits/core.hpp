#ifndef VEG_CORE_HPP_GJCBNFLAS
#define VEG_CORE_HPP_GJCBNFLAS

#include "veg/internal/macros.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {

namespace internal {
namespace meta_ {
template <bool B>
struct conditional_;
template <>
struct conditional_<true> {
	template <typename T, typename F>
	using type = T;
};
template <>
struct conditional_<false> {
	template <typename T, typename F>
	using type = F;
};
struct none {};
} // namespace meta_
} // namespace internal

namespace meta {
template <template <typename...> class F, typename... Ts>
struct meta_apply {
	using type = F<Ts...>;
};
template <typename T>
struct type_identity {
	using type = T;
};

template <bool B, typename T, typename F>
using conditional_t =
		typename internal::meta_::conditional_<B>::template type<T, F>;

template <typename T, T Value>
struct constant {
	static constexpr T value = Value;
};
template <typename T, T Value>
constexpr T constant<T, Value>::value;

template <bool B>
using bool_constant = constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename...>
using void_t = void;

template <typename... Preds>
struct disjunction;
template <typename... Preds>
struct conjunction;

template <>
struct disjunction<> : false_type {};
template <>
struct conjunction<> : true_type {};

template <typename First, typename... Preds>
struct disjunction<First, Preds...>
		: conditional_t<First::value, First, disjunction<Preds...>> {};

template <typename First, typename... Preds>
struct conjunction<First, Preds...>
		: conditional_t<First::value, conjunction<Preds...>, First> {};

} // namespace meta

namespace internal {
namespace meta_ {

template <typename T>
struct decay_helper : meta::type_identity<T> {};
template <typename Ret, typename... Args>
struct decay_helper<Ret(Args...)> : meta::type_identity<Ret (*)(Args...)> {};
template <typename T, usize N>
struct decay_helper<T[N]> : meta::type_identity<T*> {};

using namespace meta;

#if __cplusplus >= 202002L

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector {
	using value_type = bool_constant<(requires { typename Ftor<Args...>; })>;
	using type = typename conditional_t<
			value_type::value,
			meta_apply<Ftor, Args...>,
			type_identity<Default>>::type;
};

#else

template <
		typename Enable,
		typename Default,
		template <typename...>
		class Ftor,
		typename... Args>
struct _detector {
	using value_type = false_type;
	using type = Default;
};
template <typename Default, template <typename...> class Ftor, typename... Args>
struct _detector<meta::void_t<Ftor<Args...>>, Default, Ftor, Args...> {
	using value_type = true_type;
	using type = Ftor<Args...>;
};

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector : _detector<void, Default, Ftor, Args...> {};

#endif

template <template <typename...> class Op, typename... Args>
using is_detected = typename detector<meta_::none, Op, Args...>::value_type;

template <typename T>
struct is_pointer : false_type, type_identity<T> {};
template <typename T>
struct is_pointer<T*> : true_type, type_identity<T> {};

struct wrapper_base {
	static auto test(...) -> false_type;
};
template <typename T>
struct wrapper : wrapper_base {
	using wrapper_base::test;
	static auto test(wrapper<T>*) -> true_type;
};

template <typename Base>
struct baseof_wrapper : wrapper_base {
	using wrapper_base::test;
	static auto test(Base const volatile*) -> true_type;
};

template <typename T, typename U>
using is_same = decltype(wrapper<T>::test(static_cast<wrapper<U>*>(nullptr)));
template <typename Base, typename Derived>
using is_base_of =
		decltype(baseof_wrapper<Base>::test(static_cast<Derived>(nullptr)));

template <typename T>
struct is_lvalue_reference : false_type {};
template <typename T>
struct is_lvalue_reference<T&> : true_type {};
template <typename T>
struct is_rvalue_reference : false_type {};
template <typename T>
struct is_rvalue_reference<T&&> : true_type {};

template <typename T>
struct is_const : false_type {};
template <typename T>
struct is_const<T const> : true_type {};

} // namespace meta_
} // namespace internal

namespace meta {
template <typename T>
using unref_t = typename internal::meta_::unref<T&>::type;
template <typename T>
using unptr_t = typename internal::meta_::is_pointer<T>::type;

using internal::meta_::detector;

template <template <typename...> class Op, typename... Args>
using detected_t = typename internal::meta_::
		detector<internal::meta_::none, Op, Args...>::type;

template <typename T>
using decay_t = typename internal::meta_::decay_helper<unref_t<T>>::type;
} // namespace meta

namespace concepts {
using meta::conjunction;
using meta::disjunction;

#define VEG_DEF_CONCEPT_BUILTIN_OR_INTERNAL(Tpl, Name, ...)                    \
	VEG_DEF_CONCEPT(                                                             \
			Tpl,                                                                     \
			Name,                                                                    \
			VEG_HAS_BUILTIN_OR(                                                      \
					__is_##Name,                                                         \
					__is_##Name(__VA_ARGS__),                                            \
					(internal::meta_::is_##Name<__VA_ARGS__>::value)))

#if __cplusplus >= 202002L
VEG_DEF_CONCEPT(
		(template <typename...> class Op, typename... Args), detected, requires {
			typename Op<Args...>;
		});
#else
VEG_DEF_CONCEPT(
		(template <typename...> class Op, typename... Args),
		detected,
		internal::meta_::is_detected<Op, Args...>::value);
#endif

VEG_DEF_CONCEPT_BUILTIN_OR_INTERNAL((typename T, typename U), same, T, U);
VEG_DEF_CONCEPT_BUILTIN_OR_INTERNAL((typename T, typename U), base_of, T, U);

VEG_DEF_CONCEPT(
		typename T,
		const_type,
		VEG_HAS_BUILTIN_OR(
				__is_const, __is_const(T), (internal::meta_::is_const<T>::value)));
VEG_DEF_CONCEPT(
		typename T,
		void_type,
		VEG_CONCEPT(same<void const volatile, T const volatile>));

// can't use __is_pointer because of <bits/cpp_type_traits.h> header
VEG_DEF_CONCEPT(typename T, pointer, internal::meta_::is_pointer<T>::value);

VEG_DEF_CONCEPT_BUILTIN_OR_INTERNAL(typename T, lvalue_reference, T);
VEG_DEF_CONCEPT_BUILTIN_OR_INTERNAL(typename T, rvalue_reference, T);
VEG_DEF_CONCEPT_DISJUNCTION(
		typename T, reference, ((, lvalue_reference<T>), (, rvalue_reference<T>)));

} // namespace concepts
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CORE_HPP_GJCBNFLAS */
