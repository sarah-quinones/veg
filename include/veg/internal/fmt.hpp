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

struct Buffer {
	virtual void reserve(i64 new_cap) = 0;
	virtual void resize(i64 new_len) = 0;
	virtual void insert(i64 pos, char const* data, i64 len) = 0;

	VEG_NODISCARD virtual auto data() const noexcept -> char* = 0;
	VEG_NODISCARD virtual auto size() const noexcept -> i64 = 0;
};

} // namespace fmt
namespace internal {
namespace fmt {

using Buffer = veg::fmt::Buffer;

inline void to_string_impl(Buffer& out, char const* fmt, void* arg) {
	int n = abi::internal::snprintf1(nullptr, 0, fmt, arg) + 1;

	i64 old_size = out.size();
	out.resize(out.size() + n);
	abi::internal::snprintf1(out.data() + old_size, usize(n), fmt, arg);
	out.resize(old_size + n - 1);
}

struct dbg_i {
	template <typename T>
	static void to_string(Buffer& out, T arg) {
		auto _ = static_cast<long long signed>(arg);
		fmt::to_string_impl(out, "%lld", &_);
	}
};
struct dbg_u {
	template <typename T>
	static void to_string(Buffer& out, T arg) {
		auto _ = static_cast<long long unsigned>(arg);
		fmt::to_string_impl(out, "%llu", &_);
	}
};
struct dbg_f {
	template <typename T>
	static void to_string(Buffer& out, T arg) {
		auto _ = static_cast<long double>(arg);
		fmt::to_string_impl(out, "%Lf", &_);
	}
};
struct dbg_b {
	template <typename T>
	static void to_string(Buffer& out, T const& arg) {
		out.insert(out.size(), arg ? "true" : "false", arg ? 4 : 5);
	}
};
struct dbg_p {
	static void to_string(Buffer& out, void const volatile* arg) {
		auto* _ = const_cast<void*>(arg);
		fmt::to_string_impl(out, "%p", &_);
	}
};
struct dbg_pf {
	template <typename Ret, typename... Args>
	static void to_string(Buffer& out, Ret (*arg)(Args...)) {
		auto* _ = reinterpret_cast<void*>(arg);
		fmt::to_string_impl(out, "%p", &_);
	}
};
struct dbg_g {
	template <typename T>
	static void to_string(Buffer& out, T const& /*arg*/) {
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
struct Debug : internal::fmt::choose_dbg<T> {};
template <typename T>
struct Debug<T*> : internal::fmt::dbg_p {};
template <>
struct Debug<decltype(nullptr)> : internal::fmt::dbg_p {};
template <typename Ret, typename... Args>
struct Debug<Ret (*)(Args...)> : internal::fmt::dbg_pf {};

} // namespace fmt
} // namespace VEG_ABI

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
	void eprint() const noexcept;

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

inline namespace VEG_ABI {
namespace nb {
struct dbg {
	template <typename T>
	auto operator()(T&& arg) const -> T&& {
		abi::internal::String out;
		auto const& arg_c = arg;
		fmt::Debug<meta::uncvref_t<T>>::to_string(out, arg_c);
    out.eprint();
		return VEG_FWD(arg);
	}
};
} // namespace nb
VEG_NIEBLOID(dbg);
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_FMT_HPP_GQU8XFRUS */
