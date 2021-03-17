#ifndef __VEG_SIMPLE_STRING_HPP_OITPF0V1S
#define __VEG_SIMPLE_STRING_HPP_OITPF0V1S

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace assert {
namespace internal {

struct string {
	struct layout {
		char* ptr;
		i64 len;
		i64 cap;
	} self = {};

	~string();
	string() = default;
	HEDLEY_ALWAYS_INLINE string(string&& other) noexcept : self{other.self} {
		other.self = layout{nullptr, 0, 0};
	}
	string(string const&) = delete;
	auto operator=(string const&) -> string& = delete;
	auto operator=(string&&) -> string& = delete;

	void resize(i64 new_len);
	void copy(char const* data, i64 len);
	void insert(i64 pos, char const* data, i64 len);

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto data() const noexcept -> char* {
		return self.ptr;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto size() const noexcept -> i64 {
		return self.len;
	}
};
template <typename T>
auto to_string_primitive(T arg) -> assert::internal::string;
} // namespace internal
} // namespace assert
} // namespace veg

#endif /* end of include guard __VEG_SIMPLE_STRING_HPP_OITPF0V1S */
