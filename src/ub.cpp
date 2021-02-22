#include <cstring>

namespace veg {
using usize = decltype(sizeof(0));
namespace internal {
auto opaque_memcpy(void* dest, void const* src, usize n) noexcept -> void* {
  return std::memcpy(dest, src, n);
}
} // namespace internal
} // namespace veg
