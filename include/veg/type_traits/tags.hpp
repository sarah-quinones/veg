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
private:
	safety_tag_t() = default;
	constexpr operator safe_t() const noexcept { return {}; }
	friend struct meta::static_const<safety_tag_t>;
};

template <typename T>
struct tag_t {};
struct cvt_t {};
struct init_list_t {};
struct from_raw_parts_t {};
struct inplace_t {};

VEG_INLINE_VAR_TEMPLATE(typename T, tag, tag_t<T>);
VEG_INLINE_VAR(init_list, init_list_t);
VEG_INLINE_VAR(inplace, inplace_t);
VEG_INLINE_VAR(cvt, cvt_t);
VEG_INLINE_VAR(from_raw_parts, from_raw_parts_t);
VEG_INLINE_VAR(safe, safe_t);
VEG_INLINE_VAR(unsafe, unsafe_t);
} // namespace tags

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TAGS_HPP_FYDE7Q6ZS */
