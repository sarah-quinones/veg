#ifndef __VEG_CMP_HPP_6QBW4XNOS
#define __VEG_CMP_HPP_6QBW4XNOS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace internal {
namespace cmp {
enum struct which {
	int_signed_unsigned,
	int_unsigned_signed,
	floating_point,
	generic,
};

template <which Signedness /* same_sign */>
struct cmp_impl {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
			-> bool {
		return static_cast<bool>(a == b);
	}
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
			-> bool {
		return static_cast<bool>(a < b);
	}
};

template <>
struct cmp_impl<which::floating_point> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
			-> bool {
		using common = decltype(a + b);
		return static_cast<common>(a) == static_cast<common>(b);
	}
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
			-> bool {
		using common = decltype(a + b);
		return static_cast<common>(a) < static_cast<common>(b);
	}
};

template <>
struct cmp_impl<which::int_signed_unsigned> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
			-> bool {
		return (a >= 0) &&
					 (static_cast<typename std::make_unsigned<A>::type>(a) == b);
	}
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
			-> bool {
		return (a < 0) ||
					 (static_cast<typename std::make_unsigned<A>::type>(a) < b);
	}
};

template <>
struct cmp_impl<which::int_unsigned_signed> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
			-> bool {
		return cmp_impl<which::int_signed_unsigned>::eq(b, a);
	}
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto lt(A const& a, B const& b) noexcept
			-> bool {
		return (b >= 0) &&
					 (static_cast<typename std::make_unsigned<B>::type>(b) < a);
	}
};

template <typename A, typename B>
using cmp_impl_ = internal::cmp::cmp_impl<
		(VEG_CONCEPT(signed_integral<A>) &&                                    //
     VEG_CONCEPT(unsigned_integral<B>) &&                                  //
     (sizeof(A) == sizeof(B)))                                             //
				? which::int_signed_unsigned                                       //
				: (VEG_CONCEPT(unsigned_integral<A>) &&                            //
           VEG_CONCEPT(signed_integral<B>) &&                              //
           (sizeof(A) == sizeof(B)))                                       //
							? which::int_unsigned_signed                                 //
							: (VEG_CONCEPT(arithmetic<A>) && VEG_CONCEPT(arithmetic<B>)) //
										? which::floating_point                                //
										: which::generic                                       //
		>;
} // namespace cmp
} // namespace internal

namespace niebloid {
struct cmp_equal {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(equality_comparable_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return internal::cmp::cmp_impl_<A, B>::eq(a, b); }
};

struct cmp_not_equal {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(equality_comparable_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return !cmp_equal{}(a, b); }
};

struct cmp_less {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(partially_ordered_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return internal::cmp::cmp_impl_<A, B>::lt(a, b); }
};

struct cmp_greater {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(partially_ordered_with<B, A>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return cmp_less{}(b, a); }
};

struct cmp_less_equal {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(partially_ordered_with<B, A>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return !cmp_less{}(b, a); }
};

struct cmp_greater_equal {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(partially_ordered_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->bool { return !cmp_less{}(a, b); }
};

} // namespace niebloid
VEG_NIEBLOID(cmp_equal);
VEG_NIEBLOID(cmp_not_equal);
VEG_NIEBLOID(cmp_less);
VEG_NIEBLOID(cmp_greater);
VEG_NIEBLOID(cmp_less_equal);
VEG_NIEBLOID(cmp_greater_equal);
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_CMP_HPP_6QBW4XNOS */
