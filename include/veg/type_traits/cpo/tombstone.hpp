#ifndef VEG_TOMBSTONE_HPP_E6XVQKSYS
#define VEG_TOMBSTONE_HPP_E6XVQKSYS

#include "veg/type_traits/core.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace meta {
template <typename T>
struct tombstone_traits {
	static constexpr i64 spare_representations = 0;
};
template <typename T>
struct tombstone_traits<T&> {
	static constexpr i64 spare_representations = 1;
};
template <typename T>
struct tombstone_traits<T&&> {
	static constexpr i64 spare_representations = 1;
};

} // namespace meta
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TOMBSTONE_HPP_E6XVQKSYS */
