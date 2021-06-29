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
VEG_TAG_TEMPLATE(typename T, tag, Tag, T);
VEG_TAG(direct, Direct);
VEG_TAG(from_raw_parts, FromRawParts);
VEG_TAG(inplace, InPlace);
VEG_TAG(as_ref, AsRef);

VEG_TAG(safe, Safe);
VEG_TAG(unsafe, Unsafe);
} // namespace tags
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TAGS_HPP_FYDE7Q6ZS */
