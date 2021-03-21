#ifndef __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S
#define __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S

#include "veg/internal/std.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/.external/hedley.ext.h"
#include "veg/internal/.external/boostpp.ext.h"
#include "veg/internal/prologue.hpp"

#undef __VEG_PP_CAT
#define __VEG_IMPL_CAT(A, ...) A##__VA_ARGS__
#define __VEG_PP_CAT(A, ...) __VEG_IMPL_CAT(A, __VA_ARGS__)

#define __VEG_IMPL_CAT2(A, ...) A##__VA_ARGS__
#define __VEG_PP_CAT2(A, ...) __VEG_IMPL_CAT2(A, __VA_ARGS__)

#undef __VEG_PP_SEQ_TAIL
#define __VEG_PP_SEQ_TAIL(seq) __VEG_PP_CONSUME seq
#define __VEG_PP_CONSUME(x)

#define __VEG_IMPL_HEAD(arg, ...) arg
#define __VEG_IMPL_TAIL(arg, ...) __VA_ARGS__
#define __VEG_PP_HEAD(arg, ...) __VEG_IMPL_HEAD(arg, __VA_ARGS__)
#define __VEG_PP_TAIL(arg, ...) __VEG_IMPL_TAIL(arg, __VA_ARGS__)

#define __VEG_IMPL_REMOVE_PAREN1(...) __VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__
#define __VEG_IMPL_REMOVE_PAREN2(...) __VEG_PP_CAT(__VEG_IMPL_, __VA_ARGS__)
#define __VEG_IMPL___VEG_IMPL_REMOVE_PAREN1
#define __VEG_PP_REMOVE_PAREN(...)                                             \
	__VEG_IMPL_REMOVE_PAREN2(__VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__)

#define __VEG_IMPL_REMOVE_PAREN11(...) __VEG_IMPL_REMOVE_PAREN11 __VA_ARGS__
#define __VEG_IMPL_REMOVE_PAREN21(...) __VEG_PP_CAT(__VEG_IMPL_, __VA_ARGS__)
#define __VEG_IMPL___VEG_IMPL_REMOVE_PAREN11
#define __VEG_PP_REMOVE_PAREN1(...)                                            \
	__VEG_IMPL_REMOVE_PAREN21(__VEG_IMPL_REMOVE_PAREN11 __VA_ARGS__)

#define __VEG_PP_APPEND(tup, elem) (elem, __VEG_PP_REMOVE_PAREN(tup))

#define VEG_INLINE_VAR(name, obj)                                              \
	namespace /* NOLINT */ {                                                     \
	constexpr auto const& name =                                                 \
			::veg::meta::internal::static_const<obj>::value;                         \
	}                                                                            \
	VEG_NOM_SEMICOLON
#define VEG_NIEBLOID(name)                                        /* NOLINT */ \
	namespace /* NOLINT */ {                                        /* NOLINT */ \
	constexpr auto const& name =                                    /* NOLINT */ \
			::veg::meta::internal::static_const<niebloid::name>::value; /* NOLINT */ \
	}                                                               /* NOLINT */ \
	VEG_NOM_SEMICOLON                                               /* NOLINT */

#define VEG_FWD(x) static_cast<decltype(x)&&>(x)

// disallows moving const rvalues
#define VEG_MOV(x)                                                             \
	static_cast<typename ::std::remove_cv<                                       \
			typename ::std::remove_reference<decltype(x)>::type>::type&&>(x)

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
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
[[noreturn]] void terminate() noexcept;
}
} // namespace VEG_ABI_VERSION
} // namespace abi
inline namespace VEG_ABI {
// dev-only
template <typename... Ts>
struct incomplete_t;
template <typename... Types, typename... Args>
auto show_types(Args&&...) -> incomplete_t<Types..., Args...>;

namespace meta {
using std::false_type;
using std::true_type;

template <bool B>
struct bool_constant : std::integral_constant<bool, B> {};

template <bool B>
struct check_true;
template <>
struct check_true<true> : true_type {};

namespace internal {
template <typename T>
auto declval() noexcept -> T;
struct nodefault_ctor {
	nodefault_ctor() = delete;
};
struct nomove_assign {
	nomove_assign() = default;
	~nomove_assign() = default;
	nomove_assign(nomove_assign const&) = default;
	nomove_assign(nomove_assign&&) noexcept = default;
	auto operator=(nomove_assign const&) -> nomove_assign& = default;
	auto operator=(nomove_assign&&) noexcept -> nomove_assign& = delete;
};
struct nocopy_assign {
	nocopy_assign() = default;
	~nocopy_assign() = default;
	nocopy_assign(nocopy_assign const&) = default;
	nocopy_assign(nocopy_assign&&) noexcept = default;
	auto operator=(nocopy_assign const&) -> nocopy_assign& = delete;
	auto operator=(nocopy_assign&&) noexcept -> nocopy_assign& = default;
};
struct nomove_ctor {
	nomove_ctor() = default;
	~nomove_ctor() = default;
	nomove_ctor(nomove_ctor const&) = default;
	nomove_ctor(nomove_ctor&&) noexcept = delete;
	auto operator=(nomove_ctor const&) -> nomove_ctor& = default;
	auto operator=(nomove_ctor&&) noexcept -> nomove_ctor& = default;
};
struct nocopy_ctor {
	nocopy_ctor() = default;
	~nocopy_ctor() = default;
	nocopy_ctor(nocopy_ctor const&) = delete;
	nocopy_ctor(nocopy_ctor&&) noexcept = default;
	auto operator=(nocopy_ctor const&) -> nocopy_ctor& = default;
	auto operator=(nocopy_ctor&&) noexcept -> nocopy_ctor& = default;
};

template <i64 I>
struct empty_i {};

struct empty {};
using empty_arr = empty[];

template <typename T>
struct static_const {
	static constexpr T value{};
};
template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)
} // namespace internal
template <typename T>
constexpr auto max2(T a, T b) noexcept -> T {
	return a > b ? a : b;
}
template <typename T>
constexpr auto min2(T a, T b) noexcept -> T {
	return a < b ? a : b;
}

template <typename T>
constexpr auto max_of_slice(T const* arr, i64 size) noexcept -> T {
	return size == 1 ? arr[0]
									 : meta::max2(arr[0], max_of_slice(arr + 1, size - 1));
}

template <typename T>
constexpr auto max_of(std::initializer_list<T> lst) noexcept -> T {
	return meta::max_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}

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

using std::is_enum;

namespace internal {
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
} // namespace internal
template <bool B, typename T, typename F>
using conditional_t = typename internal::conditional_<B>::template type<T, F>;

template <bool B, typename T = void>
struct enable_if {
	using type = T;
};
template <typename T>
struct enable_if<false, T> {};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename U, typename V>
using discard_1st = V;

template <typename T>
using no_op = T;

// COMPAT: unused parameters are ignored, messes with SFINAE
template <typename...>
using void_t = void;

template <typename... Preds>
struct disjunction;

template <>
struct disjunction<> : false_type {};

template <typename First, typename... Preds>
struct disjunction<First, Preds...>
		: internal::conditional_<
					First::value>::template type<First, disjunction<Preds...>> {};

template <typename... Preds>
struct conjunction;

template <>
struct conjunction<> : true_type {};

template <typename First, typename... Preds>
struct conjunction<First, Preds...>
		: internal::conditional_<
					First::value>::template type<conjunction<Preds...>, First> {};

template <typename Pred>
struct negation : bool_constant<!Pred::value> {};

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

namespace internal {
// clang-format off
        template <typename T> struct uncvlref;
        template <typename T> struct uncvlref<T&> { using type = T; };
        template <typename T> struct uncvlref<T const&>  { using type = T; };
        template <typename T> struct uncvlref<T volatile&> { using type = T; };
        template <typename T> struct uncvlref<T volatile const&>  { using type = T; };

        template <typename T> struct unref ;
        template <typename T> struct unref<T&> { using type = T; };
// clang-format on
} // namespace internal

template <typename T>
using uncvref_t = typename internal::uncvlref<T&>::type;
template <typename T>
using unref_t = typename internal::unref<T&>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

enum struct category_e {
	own,
	ref,
	ref_mut,
	ref_mov,
};

namespace internal {
// clang-format off
        template <category_e C>
          struct apply_categ;
        template <> struct apply_categ<category_e::own> { template <typename T> using type = T; };
        template <> struct apply_categ<category_e::ref> { template <typename T> using type = T const&; };
        template <> struct apply_categ<category_e::ref_mut> { template <typename T> using type = T&; };
        template <> struct apply_categ<category_e::ref_mov> { template <typename T> using type = T&&; };
// clang-format on
} // namespace internal
template <category_e C, typename T>
using apply_category_t = typename internal::apply_categ<C>::template type<T>;

template <typename T>
struct value_category : std::integral_constant<category_e, category_e::own> {};

template <typename T>
struct value_category<T const&>
		: std::integral_constant<category_e, category_e::ref> {};
template <typename T>
struct value_category<T const&&>
		: std::integral_constant<category_e, category_e::ref> {};

template <typename T>
struct value_category<T&>
		: std::integral_constant<category_e, category_e::ref_mut> {};
template <typename T>
struct value_category<T&&>
		: std::integral_constant<category_e, category_e::ref_mov> {};

template <typename To, typename From>
using collapse_category_t = typename internal::apply_categ<
		value_category<From>::value>::template type<To>;

namespace internal {
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
} // namespace internal

namespace internal {
template <typename T>
struct is_pointer : false_type {};
template <typename T>
struct is_pointer<T*> : true_type {};
template <typename T>
struct is_lvalue_ref : false_type {};
template <typename T>
struct is_lvalue_ref<T&> : true_type {};
template <typename T>
struct is_rvalue_ref : false_type {};
template <typename T>
struct is_rvalue_ref<T&&> : true_type {};
} // namespace internal

template <typename T>
struct is_bounded_array : false_type {};
template <typename T, usize N>
struct is_bounded_array<T[N]> : true_type {};

template <typename T>
struct array_extent : std::integral_constant<usize, 0> {};
template <usize N, typename T>
struct array_extent<T[N]> : std::integral_constant<usize, N> {};

template <template <typename...> class F, typename... Ts>
struct meta_apply {
	using type = F<Ts...>;
};
template <typename T>
struct type_identity {
	using type = T;
};

namespace internal {
struct none {};
} // namespace internal

#if __cplusplus >= 202002L

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector {
	using found = bool_constant<(requires { typename Ftor<Args...>; })>;
	using type = typename conditional_t<
			found::value,
			meta_apply<Ftor, Args...>,
			type_identity<Default>>::type;
};

#else

namespace internal {

template <
		typename Enable,
		typename Default,
		template <typename...>
		class Ftor,
		typename... Args>
struct detector {
	using found = false_type;
	using type = Default;
};
template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector<void_t<Ftor<Args...>>, Default, Ftor, Args...> {
	using found = true_type;
	using type = Ftor<Args...>;
};

} // namespace internal

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector : internal::detector<void, Default, Ftor, Args...> {};

#endif

template <template <typename...> class Op, typename... Args>
struct is_detected : detector<internal::none, Op, Args...>::found {};

template <template <typename...> class Op, typename... Args>
using detected_t = typename detector<internal::none, Op, Args...>::type;

namespace internal {
template <typename T, typename... Args>
using uniform_ctor_expr =
		decltype(static_cast<void>(T{VEG_DECLVAL(Args &&)...}));
template <typename T, typename... Args>
using nothrow_uniform_ctor_expr =
		enable_if_t<noexcept(T{VEG_DECLVAL_NOEXCEPT(Args &&)...})>;
} // namespace internal
} // namespace meta

namespace concepts {
#if __cplusplus >= 202002L
VEG_DEF_CONCEPT(
		(template <typename...> class Op, typename... Args), detected, requires {
			typename Op<Args...>;
		});
#else
VEG_DEF_CONCEPT(
		(template <typename...> class Op, typename... Args),
		detected,
		meta::is_detected<Op, Args...>::value);
#endif
} // namespace concepts

namespace meta {
namespace internal {
template <typename Fn, typename... Args>
using call_expr = decltype(VEG_DECLVAL(Fn &&)(VEG_DECLVAL(Args &&)...));

template <typename Fn, typename... Args>
using nothrow_call_expr = enable_if_t<noexcept(
		VEG_DECLVAL_NOEXCEPT(Fn&&)(VEG_DECLVAL_NOEXCEPT(Args&&)...))>;
} // namespace internal

template <typename Fn, typename... Args>
struct invoke_result
		: detector<internal::none, internal::call_expr, Fn, Args...> {};
template <typename Fn, typename... Args>
using invoke_result_t = typename invoke_result<Fn, Args...>::type;

template <typename T>
struct value_sentinel_for : std::integral_constant<i64, 0> {
	static VEG_CPP14(constexpr) void invalid(i64 /*unused*/) noexcept;
	static VEG_CPP14(constexpr) auto id(T const& /*unused*/) noexcept -> i64;
};
} // namespace meta

using meta::uncvref_t;
using meta::unref_t;

namespace concepts {
using meta::disjunction;
using meta::conjunction;

VEG_DEF_CONCEPT(
		typename T,
		trivially_destructible,
		VEG_HAS_BUILTIN_OR(
				__has_trivial_destructor,
				(__has_trivial_destructor(T)),
				(std::is_trivially_destructible<T>::value)));

// clang-format off
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, lvalue_reference, T);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, rvalue_reference, T);

      // can't use __is_pointer because of <bits/cpp_type_traits.h> header
      VEG_DEF_CONCEPT(typename T, pointer, meta::internal::is_pointer<T>::value);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, reference, T);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, function, T);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename T, typename U), same, T, U);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename Derived, typename Base), base_of, Derived, Base);

      VEG_DEF_CONCEPT(typename T, void_type, VEG_CONCEPT(same<T const volatile, void const volatile>));
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(typename T, const_type, is_const, T);

      VEG_DEF_CONCEPT(typename T, scalar, std::is_scalar<T>::value);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, integral, T);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, floating_point, T);
      VEG_DEF_CONCEPT(typename T, signed_type, std::is_signed<T>::value);
      VEG_DEF_CONCEPT(typename T, unsigned_type, std::is_unsigned<T>::value);

      VEG_DEF_CONCEPT((typename T, typename... Args), brace_constructible, VEG_CONCEPT(detected<meta::internal::uniform_ctor_expr, T, Args&&...>));
      VEG_DEF_CONCEPT((typename T, typename... Args), nothrow_brace_constructible, VEG_CONCEPT( detected<meta::internal::nothrow_uniform_ctor_expr, T, Args&&...>));
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename T, typename... Ts), constructible, T, Ts&&...);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename T, typename... Ts), nothrow_constructible, T, Ts&&...);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename From, typename To), convertible, From&&, To);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename T, typename U), assignable, T, U);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  ((typename T, typename U), nothrow_assignable, T&&, U&&);

      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD  (typename T, trivially_copyable, T);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(typename T, trivially_copy_constructible, is_trivially_constructible, T, T const&);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(typename T, trivially_move_constructible, is_trivially_constructible, T, T&&);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(typename T, trivially_copy_assignable, is_trivially_assignable, T&, T const&);
      VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(typename T, trivially_move_assignable, is_trivially_assignable, T&, T&&);

      VEG_DEF_CONCEPT(typename T, move_constructible, VEG_CONCEPT(constructible<T, T&&>));
      VEG_DEF_CONCEPT(typename T, nothrow_move_constructible, VEG_CONCEPT(nothrow_constructible<T, T&&>));
      VEG_DEF_CONCEPT(typename T, move_assignable, VEG_CONCEPT(assignable<T&, T&&>));
      VEG_DEF_CONCEPT(typename T, nothrow_move_assignable, VEG_CONCEPT(nothrow_assignable<T&, T&&>));

      VEG_DEF_CONCEPT(typename T, copy_constructible, VEG_CONCEPT(constructible<T, T const&>));
      VEG_DEF_CONCEPT(typename T, nothrow_copy_constructible, VEG_CONCEPT(nothrow_constructible<T, T const&>));
      VEG_DEF_CONCEPT(typename T, copy_assignable, VEG_CONCEPT(assignable<T&, T const&>));
      VEG_DEF_CONCEPT(typename T, nothrow_copy_assignable, VEG_CONCEPT(nothrow_assignable<T&, T const&>));
      VEG_DEF_CONCEPT((typename Fn, typename... Args), invocable, VEG_CONCEPT(detected<meta::internal::call_expr, Fn, Args&&...>));
      VEG_DEF_CONCEPT((typename Fn, typename... Args), nothrow_invocable, VEG_CONCEPT(detected<meta::internal::nothrow_call_expr, Fn, Args&&...>));

      VEG_DEF_CONCEPT_CONJUNCTION(typename T, signed_integral, ((, integral<T>), (, signed_type<T>)));
      VEG_DEF_CONCEPT_CONJUNCTION(typename T, unsigned_integral, ((, integral<T>), (, unsigned_type<T>)));
      VEG_DEF_CONCEPT_DISJUNCTION(typename T, arithmetic, ((, integral<T>), (, floating_point<T>)));
// clang-format on

} // namespace concepts

namespace niebloid {
struct invoke {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(VEG_CONCEPT(invocable<Fn, Args&&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&),
			(... args, Args&&))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>))
			->meta::invoke_result_t<Fn, Args&&...> {
		return VEG_FWD(fn)(VEG_FWD(args)...);
	}
};
} // namespace niebloid
VEG_NIEBLOID(invoke);

template <typename T>
struct tag_t {};

enum struct safety_e { safe, unsafe };

template <safety_e S>
struct safety_tag_t;

using safe_t = safety_tag_t<safety_e::safe>;
using unsafe_t = safety_tag_t<safety_e::unsafe>;

template <>
struct safety_tag_t<safety_e::safe>
		: std::integral_constant<safety_e, safety_e::safe> {};

namespace internal {
template <typename T>
struct make_unsafe;
} // namespace internal

template <>
struct safety_tag_t<safety_e::unsafe>
		: private std::integral_constant<safety_e, safety_e::unsafe> {

	using std::integral_constant<safety_e, safety_e::unsafe>::value;
	using typename std::integral_constant<safety_e, safety_e::unsafe>::value_type;
	using typename std::integral_constant<safety_e, safety_e::unsafe>::type;
	using std::integral_constant<safety_e, safety_e::unsafe>::operator value_type;

	constexpr
	operator safe_t() const noexcept // NOLINT(hicpp-explicit-conversions)
	{
		return {};
	}

private:
	friend struct internal::make_unsafe<void>;
	constexpr safety_tag_t() = default;
};

namespace internal {
template <typename T>
struct make_unsafe {
	static constexpr unsafe_t value = {};
};
template <typename T>
constexpr unsafe_t make_unsafe<T>::value; // NOLINT
} // namespace internal

namespace { // NOLINT(cert-dcl59-cpp)
constexpr auto const& unsafe /* NOLINT */ = internal::make_unsafe<void>::value;
} // namespace
VEG_INLINE_VAR(safe, safe_t);

namespace meta {
namespace internal {

template <typename A, typename B>
using cmp_eq = decltype(
		void(static_cast<bool>(VEG_DECLVAL(A const&) == VEG_DECLVAL(B const&))));
template <typename A, typename B>
using cmp_lt = decltype(
		void(static_cast<bool>(VEG_DECLVAL(A const&) < VEG_DECLVAL(B const&))));

template <typename A, typename B>
struct equality_comparable_impl
		: bool_constant<
					(VEG_CONCEPT(same<A, B>) || (!std::is_enum<A>::value &&   //
                                       !std::is_enum<B>::value)) && //
					is_detected<cmp_eq, A, B>::value> {};

template <typename A, typename B>
struct less_than_comparable_impl
		: bool_constant<
					(VEG_CONCEPT(same<A, B>) || (!std::is_enum<A>::value &&   //
                                       !std::is_enum<B>::value)) && //
					is_detected<cmp_lt, A, B>::value> {};

} // namespace internal
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(
		(typename A, typename B),
		equality_comparable_with,
		meta::internal::equality_comparable_impl<uncvref_t<A>, uncvref_t<B>>::
				value);

VEG_DEF_CONCEPT(
		(typename A, typename B),
		partially_ordered_with,
		meta::internal::less_than_comparable_impl<uncvref_t<A>, uncvref_t<B>>::
				value);
} // namespace concepts

namespace meta {
namespace internal {

struct adl_fn_swap {
	template <typename U, typename V>
	using type = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));

	template <typename U, typename V>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(
			U&& u, V&& v) noexcept(noexcept(swap(VEG_FWD(u), VEG_FWD(v)))) {
		swap(VEG_FWD(u), (VEG_FWD(v)));
	}
};
struct mov_fn_swap {
	template <typename U, typename V>
	using type = void;

	template <typename U>
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void apply(U& u, U& v) noexcept(
			(VEG_CONCEPT(nothrow_move_constructible<U>) &&
	     VEG_CONCEPT(nothrow_move_assignable<U>))) {
		auto tmp = static_cast<U&&>(u);
		u = static_cast<U&&>(v);
		v = static_cast<U&&>(tmp);
	}
};

template <typename U, typename V>
struct has_adl_swap : meta::is_detected<adl_fn_swap::type, U, V>,
											adl_fn_swap {};

template <typename U, typename V>
struct has_mov_swap : meta::false_type {};
template <typename U>
struct has_mov_swap<U&, U&> : meta::bool_constant<
																	VEG_CONCEPT(move_constructible<U>) &&
																	VEG_CONCEPT(move_assignable<U>)>,
															mov_fn_swap {};

template <typename U, typename V>
struct swap_impl : meta::disjunction<has_adl_swap<U, V>, has_mov_swap<U, V>> {};

template <typename U, typename V>
using nothrow_swap_expr = enable_if_t<noexcept(swap_impl<U, V>::apply(
		VEG_DECLVAL_NOEXCEPT(U&&), VEG_DECLVAL_NOEXCEPT(V&&)))>;

template <typename U, typename V>
using adl_swap_expr = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));
} // namespace internal
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(
		(typename U, typename V),
		adl_swappable,
		VEG_CONCEPT(detected<meta::internal::adl_swap_expr, U&&, V&&>));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename U, typename V),
		move_swappable,
		((, same<U, V>),
     (, lvalue_reference<U>),
     (, move_constructible<unref_t<U>>),
     (, move_assignable<unref_t<U>>)));

VEG_DEF_CONCEPT_DISJUNCTION(
		(typename U, typename V),
		swappable,
		((, adl_swappable<U, V>), (, move_swappable<U, V>)));

VEG_DEF_CONCEPT(
		(typename U, typename V),
		nothrow_swappable,
		VEG_CONCEPT(detected<meta::internal::nothrow_swap_expr, U&&, V&&>));
} // namespace concepts

namespace niebloid {
struct swap {
	VEG_TEMPLATE(
			(typename U, typename V),
			requires(VEG_CONCEPT(swappable<U&&, V&&>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void
			operator(),
			(u, U&&),
			(v, V&&))
	const noexcept(VEG_CONCEPT(nothrow_swappable<U&&, V&&>)) {
		meta::internal::swap_impl<U&&, V&&>::apply(VEG_FWD(u), VEG_FWD(v));
	}
};
} // namespace niebloid
VEG_NIEBLOID(swap);

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
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&>))) {
		VEG_FWD(fn)();
	}
};
VEG_CPP17(

		template <typename Fn> defer(Fn) -> defer<Fn>;

)

namespace make {
namespace niebloid {
struct defer {
	VEG_TEMPLATE(
			typename Fn,
			requires(
					VEG_CONCEPT(move_constructible<Fn>) && VEG_CONCEPT(invocable<Fn>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) auto
			operator(),
			(fn, Fn))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))->veg::defer<Fn> {
		return {VEG_FWD(fn)};
	}
};
} // namespace niebloid
VEG_NIEBLOID(defer);
} // namespace make

namespace meta {
// unsafe trait: trivial copy/move ctor/assignment, trivial dtor,
// default ctor (value initialization) must not have any observable side effects
// (usually sets to zero)
template <typename T>
struct is_mostly_trivial : VEG_HAS_BUILTIN_OR(
															 __is_trivial,
															 (bool_constant<__is_trivial(T)>),
															 (std::is_trivial<T>)) {};

template <typename T>
struct is_trivially_relocatable
		: bool_constant<
					VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(trivially_move_constructible<T>)> {};

template <typename U>
struct is_trivially_swappable : false_type {};

template <typename T>
struct is_trivially_swappable<T&>
		: bool_constant<(
					VEG_CONCEPT(trivially_copyable<T>) &&        //
					VEG_CONCEPT(trivially_move_assignable<T>) && //
					VEG_CONCEPT(trivially_move_constructible<T>) &&
					!VEG_CONCEPT(adl_swappable<T&, T&>)) //
                    > {};

} // namespace meta

namespace concepts {
// clang-format off
      VEG_DEF_CONCEPT(typename T, mostly_trivial, meta::is_mostly_trivial<T>::value);
      VEG_DEF_CONCEPT( typename T, trivially_relocatable, meta::is_trivially_relocatable<T>::value);
      VEG_DEF_CONCEPT( typename T, trivially_swappable, meta::is_trivially_swappable<T>::value);
// clang-format on
} // namespace concepts

namespace meta {
namespace internal {

template <typename T>
void get() = delete;

struct array_get {
	template <typename I, typename T>
	using result_type = decltype(VEG_DECLVAL(T &&)[I::value]);

	template <usize I, typename T>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arr) noexcept
			-> decltype(VEG_FWD(arr)[I]) {
		return VEG_FWD(arr)[I];
	}
};

struct member_get {
	template <typename I, typename T>
	using result_type = decltype(VEG_DECLVAL(T &&).template get<I::value>());
	template <usize I, typename T>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arg)
			VEG_DEDUCE_RET(VEG_FWD(arg).template get<I>());
};
struct adl_get {
	template <typename I, typename T>
	using result_type = decltype(get<I::value>(VEG_DECLVAL(T &&)));

	template <usize I, typename T>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arg)
			VEG_DEDUCE_RET(get<I>(VEG_FWD(arg)));
};

struct none_found : true_type {
	template <typename I, typename T>
	using result_type = void;
};

template <typename I, typename T>
using member_get_expr = decltype(VEG_DECLVAL(T &&).template get<I::value>());
template <typename I, typename T>
using adl_get_expr = decltype(get<I::value>(VEG_DECLVAL(T &&)));

template <usize I, typename T>
struct has_array_get
		: meta::bool_constant<meta::is_bounded_array<meta::uncvref_t<T>>::value>,
			array_get {};

template <usize I, typename T>
struct has_member_get : meta::is_detected<
														member_get::result_type,
														std::integral_constant<usize, I>,
														T&&>,
												member_get {};
template <usize I, typename T>
struct has_adl_get : meta::is_detected<
												 adl_get::result_type,
												 std::integral_constant<usize, I>,
												 T&&>,
										 adl_get {};

} // namespace internal
} // namespace meta

namespace concepts {
namespace aux {
VEG_DEF_CONCEPT((usize I, usize N), less_than, (I < N));
VEG_DEF_CONCEPT(typename T, bounded_array, meta::is_bounded_array<T>::value);
} // namespace aux
VEG_DEF_CONCEPT_CONJUNCTION(
		(usize I, typename T),
		array_gettable,
		((aux::, bounded_array<uncvref_t<T>>),
     (aux::, less_than<I, meta::array_extent<uncvref_t<T>>::value>)));
VEG_DEF_CONCEPT(
		(usize I, typename T),
		member_gettable,
		VEG_CONCEPT(detected<
								meta::internal::member_get_expr,
								std::integral_constant<usize, I>,
								T>));
VEG_DEF_CONCEPT(
		(usize I, typename T),
		adl_gettable,
		VEG_CONCEPT(detected<
								meta::internal::adl_get_expr,
								std::integral_constant<usize, I>,
								T>));

VEG_DEF_CONCEPT_DISJUNCTION(
		(usize I, typename T),
		gettable,
		((, array_gettable<I, T>), //
     (, member_gettable<I, T>),
     (, adl_gettable<I, T>)));
} // namespace concepts

namespace niebloid {
template <usize I>
struct get {
	VEG_TEMPLATE(
			(typename T,
	     typename Impl = meta::disjunction<
					 meta::internal::has_array_get<I, T>,
					 meta::internal::has_member_get<I, T>,
					 meta::internal::has_adl_get<I, T>,
					 meta::internal::none_found>),
			requires(VEG_CONCEPT(gettable<I, T>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(arg, T&&))
	const noexcept(noexcept(Impl::template apply<I>(VEG_FWD(arg))))
			->meta::detected_t<
					Impl::template result_type,
					std::integral_constant<usize, I>,
					T> {
		return Impl::template apply<I>(VEG_FWD(arg));
	}
};
} // namespace niebloid

template <typename... Ts>
struct tuple;
template <typename T>
struct option;

inline namespace tags {
struct elems_t {};
struct init_list_t {};
struct from_raw_parts_t {};
struct inplace_t {};
struct none_t {
	friend constexpr auto operator==(none_t /*lhs*/, none_t /*rhs*/) noexcept
			-> bool {
		return true;
	}
	friend constexpr auto operator!=(none_t /*lhs*/, none_t /*rhs*/) noexcept
			-> bool {
		return false;
	}

private:
	constexpr none_t() = default;
	constexpr explicit none_t(none_t* /*unused*/) noexcept {}
	template <typename T>
	friend struct meta::internal::static_const;
};
struct some_t {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(constructible<meta::uncvref_t<T>, T&&>)),
			VEG_CPP14(constexpr) auto
			operator(),
			(arg, T&&))
	const noexcept(VEG_CONCEPT(nothrow_constructible<meta::uncvref_t<T>, T&&>))
			->option<meta::uncvref_t<T>> {
		return {*this, VEG_FWD(arg)};
	}

private:
	constexpr some_t() = default;
	constexpr explicit some_t(some_t* /*unused*/) noexcept {}
	template <typename T>
	friend struct meta::internal::static_const;
};

VEG_IGNORE_CPP14_EXTENSION_WARNING(
		namespace { /* NOLINT */
								template <usize I>
								constexpr auto const& get /* NOLINT */ =
										::veg::meta::internal::static_const<
												niebloid::get<I>>::value;

								template <typename T>
								constexpr auto const& tag =
										::veg::meta::internal::static_const<tag_t<T>>::value;
    } // namespace
)

VEG_INLINE_VAR(some, some_t);
VEG_INLINE_VAR(init_list, init_list_t);
VEG_INLINE_VAR(inplace, inplace_t);
VEG_INLINE_VAR(elems, elems_t);
VEG_INLINE_VAR(none, none_t);
VEG_INLINE_VAR(from_raw_parts, from_raw_parts_t);
} // namespace tags

namespace int_c {
struct dyn;
template <i64 N>
struct fix;
} // namespace int_c

namespace meta {
namespace internal {
template <typename T>
struct is_fix : std::false_type {};
template <i64 N>
struct is_fix<int_c::fix<N>> : std::true_type {};
} // namespace internal
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(
		typename T,
		meta_int,
		VEG_CONCEPT(same<T, int_c::dyn>) || meta::internal::is_fix<T>::value);
} // namespace concepts
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */
