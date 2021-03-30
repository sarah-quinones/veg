#ifndef VEG_TAGS_HPP_FYDE7Q6ZS
#define VEG_TAGS_HPP_FYDE7Q6ZS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {

namespace internal {
template <typename T>
struct make_unsafe;
} // namespace internal

inline namespace tags {
enum struct safety_e { safe, unsafe };

template <safety_e S>
struct safety_tag_t;

using safe_t = safety_tag_t<safety_e::safe>;
using unsafe_t = safety_tag_t<safety_e::unsafe>;

template <>
struct safety_tag_t<safety_e::safe> : meta::constant<safety_e, safety_e::safe> {
};

template <>
struct safety_tag_t<safety_e::unsafe>
		: meta::constant<safety_e, safety_e::unsafe> {
	constexpr operator safe_t() const noexcept { return {}; }

private:
	friend struct internal::make_unsafe<void>;
	constexpr safety_tag_t() = default;
};
} // namespace tags

namespace internal {
template <typename T>
struct make_unsafe {
	static constexpr unsafe_t value = {};
};
template <typename T>
constexpr unsafe_t make_unsafe<T>::value;
} // namespace internal

inline namespace tags {
namespace {
constexpr auto const& unsafe /* NOLINT */ = internal::make_unsafe<void>::value;
} // namespace

template <typename T>
struct tag_t {};
struct cvt_t {};
struct init_list_t {};
struct from_raw_parts_t {};
struct inplace_t {};

VEG_IGNORE_CPP14_EXTENSION_WARNING(namespace {
	template <typename T>
	constexpr auto const& tag = internal::meta_::static_const<tag_t<T>>::value;
} // namespace
)

VEG_INLINE_VAR(init_list, init_list_t);
VEG_INLINE_VAR(inplace, inplace_t);
VEG_INLINE_VAR(cvt, cvt_t);
VEG_INLINE_VAR(from_raw_parts, from_raw_parts_t);
VEG_INLINE_VAR(safe, safe_t);
} // namespace tags

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TAGS_HPP_FYDE7Q6ZS */
