#ifndef VEG_BYTE_STRING_REF_HPP_NK1AVIX7S
#define VEG_BYTE_STRING_REF_HPP_NK1AVIX7S

#include "veg/type_traits/constructible.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
struct char_string_ref {
	HEDLEY_ALWAYS_INLINE constexpr char_string_ref(
			char const* data, i64 len) noexcept
			: data_{data}, len_{len} {}

	char_string_ref(char const* str) noexcept;

	VEG_TEMPLATE(
			typename T,
			requires(
					VEG_CONCEPT(constructible<
											char const*,
											decltype(VEG_DECLVAL(T const&).data())>) &&
					VEG_CONCEPT(
							constructible<i64, decltype(VEG_DECLVAL(T const&).size())>)),

			HEDLEY_ALWAYS_INLINE constexpr,
			char_string_ref,
			(arg, T const&))
	noexcept
			: char_string_ref{
						static_cast<char const*>(arg.data()),
						static_cast<i64>(arg.size())} {}

	char const* data_;
	i64 len_;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto data() const noexcept
			-> char const* {
		return data_;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto size() const noexcept
			-> i64 {
		return len_;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto
	starts_with(char_string_ref other) const noexcept -> bool;
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto
	operator==(char_string_ref other) const noexcept -> bool;
};
template <typename T>
struct byte_str_static_const {
	static constexpr T value = {"", 0};
};
template <typename T>
constexpr T byte_str_static_const<T>::value;
namespace /* NOLINT */ {
constexpr auto const& empty_str /* NOLINT */ =
		byte_str_static_const<char_string_ref>::value;
} // namespace
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_BYTE_STRING_REF_HPP_NK1AVIX7S */
