#ifndef VEG_MACROS_HPP_HSTLSKZXS
#define VEG_MACROS_HPP_HSTLSKZXS
#include "veg/internal/.external/hedley.ext.h"
#include <initializer_list>
#include "veg/internal/typedefs.hpp"
#include "veg/internal/preprocessor.hpp"
#include "veg/internal/prologue.hpp"

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

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_MACROS_HPP_HSTLSKZXS */
