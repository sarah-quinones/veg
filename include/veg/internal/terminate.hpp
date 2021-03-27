#ifndef VEG_TERMINATE_HPP_YMTONE4HS
#define VEG_TERMINATE_HPP_YMTONE4HS

#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
[[noreturn]] void terminate() noexcept;
}
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TERMINATE_HPP_YMTONE4HS */
