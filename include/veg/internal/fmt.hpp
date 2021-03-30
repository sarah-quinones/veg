#ifndef VEG_FMT_HPP_GQU8XFRUS
#define VEG_FMT_HPP_GQU8XFRUS

#include "veg/type_traits/primitives.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto snprintf1(char* out, usize n, char const* fmt, void* arg) -> int;
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

inline void to_string_impl(buffer& out, char const* fmt, void* arg) {
	int n = abi::internal::snprintf1(nullptr, 0, fmt, arg) + 1;

	i64 old_size = out.size();
	out.resize(out.size() + n);
	abi::internal::snprintf1(out.data() + old_size, usize(n), fmt, arg);
	out.resize(old_size + n - 1);
}

struct dbg_i {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		auto _ = static_cast<long long signed>(arg);
		fmt::to_string_impl(out, "%lld", &_);
	}
};
struct dbg_u {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		auto _ = static_cast<long long unsigned>(arg);
		fmt::to_string_impl(out, "%llu", &_);
	}
};
struct dbg_f {
	template <typename T>
	static void to_string(buffer& out, T arg) {
		auto _ = static_cast<long double>(arg);
		fmt::to_string_impl(out, "%Lf", &_);
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
		auto* _ = const_cast<void*>(arg);
		fmt::to_string_impl(out, "%p", &_);
	}
};
struct dbg_pf {
	template <typename Ret, typename... Args>
	static void to_string(buffer& out, Ret (*arg)(Args...)) {
		auto* _ = reinterpret_cast<void*>(arg);
		fmt::to_string_impl(out, "%p", &_);
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
template <typename Ret, typename... Args>
struct debug<Ret (*)(Args...)> : internal::fmt::dbg_pf {};

} // namespace fmt
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_FMT_HPP_GQU8XFRUS */