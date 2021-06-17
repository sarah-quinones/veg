#ifndef VEG_TAGS_HPP_FYDE7Q6ZS
#define VEG_TAGS_HPP_FYDE7Q6ZS

#include "veg/type_traits/core.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
template <typename T>
struct make_unsafe;
} // namespace internal

inline namespace tags {
enum struct safety_e { safe, unsafe };

template <safety_e S>
struct SafetyTag;

using Safe = SafetyTag<safety_e::safe>;
using Unsafe = SafetyTag<safety_e::unsafe>;

template <>
struct SafetyTag<safety_e::safe> : meta::constant<safety_e, safety_e::safe> {};

template <>
struct SafetyTag<safety_e::unsafe>
		: meta::constant<safety_e, safety_e::unsafe> {
private:
	SafetyTag() = default;
	constexpr operator Safe() const VEG_ALWAYS_NOEXCEPT { return {}; }
	friend struct meta::static_const<SafetyTag>;
};

template <typename T>
struct Tag {};
struct Cvt {};
struct Direct {};
struct Into {};
struct InitList {};
struct FromRawParts {};
struct InPlace {};
struct AsRef {};

VEG_INLINE_VAR_TEMPLATE(typename T, tag, Tag<T>);
VEG_INLINE_VAR(init_list, InitList);
VEG_INLINE_VAR(inplace, InPlace);
VEG_INLINE_VAR(direct, Direct);
VEG_INLINE_VAR(safe, Safe);
VEG_INLINE_VAR(unsafe, Unsafe);
VEG_INLINE_VAR(as_ref, AsRef);
VEG_INLINE_VAR(from_raw_parts, FromRawParts);
} // namespace tags
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TAGS_HPP_FYDE7Q6ZS */
