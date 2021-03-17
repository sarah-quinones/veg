#ifndef __VEG_BEGIN_END_HPP_LH1IYIZBS
#define __VEG_BEGIN_END_HPP_LH1IYIZBS

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace legacy {

namespace internal {
namespace begin_end {

struct member_begin_end {
	template <typename T>
	using type =
			decltype((void(__VEG_DECLVAL(T).begin()), void(__VEG_DECLVAL(T).end())));
	template <typename T>
	static constexpr auto b(T&& arg) noexcept(noexcept(VEG_FWD(arg).begin()))
			-> decltype(auto) {
		return VEG_FWD(arg).begin();
	}
	template <typename T>
	static constexpr auto e(T&& arg) noexcept(noexcept(VEG_FWD(arg).end()))
			-> decltype(auto) {
		return VEG_FWD(arg).end();
	}
};
struct adl_begin_end {
	template <typename T>
	using type =
			decltype((void(begin(__VEG_DECLVAL(T))), void(end(__VEG_DECLVAL(T)))));
	template <typename T>
	static constexpr auto b(T&& arg) noexcept(noexcept(begin(VEG_FWD(arg))))
			-> decltype(auto) {
		return begin(VEG_FWD(arg));
	}
	template <typename T>
	static constexpr auto e(T&& arg) noexcept(noexcept(end(VEG_FWD(arg))))
			-> decltype(auto) {
		return end(VEG_FWD(arg));
	}
};
struct array_begin_end {
	template <typename T>
	static constexpr auto b(T&& arg) noexcept -> decltype(auto) {
		return +arg;
	}
	template <typename T>
	static constexpr auto e(T&& arg) noexcept -> decltype(auto) {
		return arg + sizeof(arg) / sizeof(arg[0]);
	}
};

template <typename T>
struct has_member_begin_end : meta::is_detected<member_begin_end::type, T&&>,
															member_begin_end {};
template <typename T>
struct has_adl_begin_end : meta::is_detected<adl_begin_end::type, T&&>,
													 adl_begin_end {};
template <typename T>
struct has_array_begin_end : meta::bounded_array<meta::uncvref_t<T>>,
														 array_begin_end {};

template <typename T>
struct has_begin_end : meta::disjunction<
													 has_member_begin_end<T>,
													 has_adl_begin_end<T>,
													 has_array_begin_end<T>> {};

} // namespace begin_end
} // namespace internal

namespace tt {
template <typename T>
struct has_begin : internal::begin_end::has_begin_end<T&&> {};
template <typename T>
struct has_end : has_begin<T> {};
} // namespace tt

namespace fn {
struct begin {
	VEG_TEMPLATE(
			(typename T),
			requires tt::has_begin<T>::value,
			constexpr auto
			operator(),
			(rng, T&&))
	const noexcept(
			noexcept(internal::begin_end::has_begin_end<T&&>::b(VEG_FWD(rng))))
			->decltype(auto) {
		return internal::begin_end::has_begin_end<T&&>::b(VEG_FWD(rng));
	}
};
struct end {
	VEG_TEMPLATE(
			(typename T),
			requires tt::has_end<T>::value,
			constexpr auto
			operator(),
			(rng, T&&))
	const noexcept(
			noexcept(internal::begin_end::has_begin_end<T&&>::e(VEG_FWD(rng))))
			->decltype(auto) {
		return internal::begin_end::has_begin_end<T&&>::e(VEG_FWD(rng));
	}
};
} // namespace fn
__VEG_ODR_VAR(begin, fn::begin);
__VEG_ODR_VAR(end, fn::end);

} // namespace legacy
} // namespace veg

#endif /* end of include guard __VEG_BEGIN_END_HPP_LH1IYIZBS */
