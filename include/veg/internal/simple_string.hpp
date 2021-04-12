#ifndef VEG_SIMPLE_STRING_HPP_OITPF0V1S
#define VEG_SIMPLE_STRING_HPP_OITPF0V1S

#include "veg/internal/fmt.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
struct String : fmt::Buffer {
	struct layout {
		char* ptr;
		i64 len;
		i64 cap;
	} self = {};

	~String();
	String() = default;
	HEDLEY_ALWAYS_INLINE String(String&& other) noexcept : self{other.self} {
		other.self = layout{nullptr, 0, 0};
	}
	String(String const&) = delete;
	auto operator=(String const&) -> String& = delete;
	auto operator=(String&&) -> String& = delete;

	void resize(i64 new_len) override;
	void reserve(i64 new_cap) override;
	void insert(i64 pos, char const* data, i64 len) override;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto data() const noexcept
			-> char* override {
		return self.ptr;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto size() const noexcept
			-> i64 override {
		return self.len;
	}
};
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_SIMPLE_STRING_HPP_OITPF0V1S */
