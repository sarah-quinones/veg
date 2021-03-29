#ifndef VEG_ADDRESS_HPP_ZP6FDIHZS
#define VEG_ADDRESS_HPP_ZP6FDIHZS

#include "veg/type_traits/core.hpp"
#include "veg/internal/std.hpp"
#include "veg/internal/prologue.hpp"

#if !VEG_HAS_BUILTIN(__builtin_addressof)

// STD INDCLUDE: addressof
#if __VEG_HAS_INCLUDE(<bits/move.h>)
#include <bits/move.h>
#else
#include <memory>
#endif
#endif

namespace veg {
inline namespace VEG_ABI {

#if !(VEG_HAS_BUILTIN(__builtin_addressof) || __cplusplus >= 201703L)

namespace internal {
namespace mem_ {
struct member_addr {
	template <typename T>
	using type = decltype(void(VEG_DECLVAL(T&).operator&()));

	template <typename T>
	HEDLEY_ALWAYS_INLINE static auto apply(T& var) noexcept -> T* {
		using char_ref = char&;
		return static_cast<T*>(static_cast<void*>(&char_ref(var)));
	}
};
struct adl_addr : member_addr {
	template <typename T>
	using type = decltype(void(operator&(VEG_DECLVAL(T&))));
};
struct builtin_addr : std::true_type {
	template <typename T>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(T& var) noexcept -> T* {
		return &var;
	}
};
template <typename T>
struct has_member_addr : meta::is_detected<member_addr::type, T&>,
												 member_addr {};
template <typename T>
struct has_adl_addr : meta::is_detected<adl_addr::type, T&>, adl_addr {};

template <typename T>
struct addr_impl
		: meta::disjunction<has_member_addr<T>, has_adl_addr<T>, builtin_addr> {};

} // namespace mem_
} // namespace internal

#endif

namespace mem {
namespace nb {
struct addressof {
	template <typename T>
	constexpr void operator()(T const&& var) const noexcept = delete;

	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T& var) const noexcept -> T* {
#if VEG_HAS_BUILTIN(__builtin_addressof)
		return __builtin_addressof(var);
#elif __cplusplus >= 201703L
		return ::std::addressof(var);
#else
		return internal::mem_::addr_impl<T>::apply(var);
#endif
	}
};
} // namespace nb
VEG_NIEBLOID(addressof);
} // namespace mem
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ADDRESS_HPP_ZP6FDIHZS */
