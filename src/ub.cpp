#include <cstring>

namespace veg {
namespace internal {
namespace memory {
using usize = decltype(sizeof(0));
auto opaque_memmove(void* dest, void const* src, usize n) noexcept -> void* {
  if (n > 0) {
    std::memmove(dest, src, n);
  }
  return dest;
}
} // namespace memory
} // namespace internal
} // namespace veg
