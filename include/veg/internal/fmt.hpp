#ifndef __VEG_FMT_HPP_GQU8XFRUS
#define __VEG_FMT_HPP_GQU8XFRUS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/prologue.hpp"
#include <cstdarg>

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto vsnprintf(char* out, usize n, char const* fmt, va_list args) -> int;
}
} // namespace VEG_ABI_VERSION
} // namespace abi

inline namespace VEG_ABI {
namespace fmt {

struct buffer {
	virtual void reserve(i64 new_cap) = 0;
	virtual void resize(i64 new_len) = 0;
	virtual void insert(i64 pos, char const* data, i64 len) = 0;

	VEG_NODISCARD virtual auto data() const noexcept -> char* = 0;
	VEG_NODISCARD virtual auto size() const noexcept -> i64 = 0;
};

} // namespace fmt
namespace internal {
namespace fmt {

using buffer = veg::fmt::buffer;

inline void to_string_impl(buffer& out, char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int n = abi::internal::vsnprintf(nullptr, 0, fmt, args) + 1;
	va_end(args);

	va_start(args, fmt);
	i64 old_size = out.size();
	out.resize(out.size() + n + 1);
	abi::internal::vsnprintf(out.data() + old_size, usize(n + 1), fmt, args);
	out.resize(n - 1);
	va_end(args);
}

struct dbg_i {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		fmt::to_string_impl(out, "%lld", static_cast<long long signed>(arg));
	}
};
struct dbg_u {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		fmt::to_string_impl(out, "%llu", static_cast<long long unsigned>(arg));
	}
};
struct dbg_f {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		fmt::to_string_impl(out, "%Lf", static_cast<long double>(arg));
	}
};
struct dbg_b {
	template <typename T>
	static void to_string(buffer& out, T const& arg) {
		out.insert(out.size(), arg ? "true" : "false", arg ? 4 : 5);
	}
};
struct dbg_p {
	static void to_string(buffer& out, void const volatile* arg) {
		fmt::to_string_impl(out, "%p", const_cast<void*>(arg));
	}
};
struct dbg_g {
	template <typename T>
	static void to_string(buffer& out, T const& /*arg*/) {
		out.insert(out.size(), "{?}", 3);
	}
};

template <typename T>
using choose_dbg = meta::conditional_t<
		VEG_CONCEPT(same<T, bool>),
		internal::fmt::dbg_b,
		meta::conditional_t<
				VEG_CONCEPT(signed_integral<T>),
				internal::fmt::dbg_i,
				meta::conditional_t<
						VEG_CONCEPT(unsigned_integral<T>),
						internal::fmt::dbg_u,
						meta::conditional_t<
								VEG_CONCEPT(floating_point<T>),
								internal::fmt::dbg_f,
								meta::conditional_t<
										VEG_CONCEPT(constructible<bool, T>),
										internal::fmt::dbg_b,
										internal::fmt::dbg_g>>>>>;

} // namespace fmt
} // namespace internal

namespace fmt {

template <typename T>
struct debug : internal::fmt::choose_dbg<T> {};
template <typename T>
struct debug<T*> : internal::fmt::dbg_p {};
template <>
struct debug<decltype(nullptr)> : internal::fmt::dbg_p {};

} // namespace fmt
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_FMT_HPP_GQU8XFRUS */
