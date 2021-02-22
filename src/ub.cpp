#include <cstring>

namespace veg {
using usize = decltype(sizeof(0));
namespace internal {
auto opaque_memmove(void* dest, void const* src, usize n) noexcept -> void* {
  if (n > 0) {
    std::memmove(dest, src, n);
  }
  return dest;
}
} // namespace internal
} // namespace veg
