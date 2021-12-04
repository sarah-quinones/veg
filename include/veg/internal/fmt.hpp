#ifndef VEG_FMT_HPP_GQU8XFRUS
#define VEG_FMT_HPP_GQU8XFRUS

#include "veg/type_traits/primitives.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/ref.hpp"
#include "veg/internal/prologue.hpp"
#include <cstring>

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto snprintf1(char* out, usize n, unsigned type, void* arg) -> usize;
}
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace fmt {
template <typename T>
struct Debug;

struct Buffer {
	usize indent_level = 0;

	Buffer() = default;
	Buffer(Buffer const&) = delete;
	Buffer(Buffer&&) = delete;
	auto operator=(Buffer const&) -> Buffer& = delete;
	auto operator=(Buffer&&) -> Buffer& = delete;
	virtual void reserve(usize new_cap) = 0;
	virtual void resize(usize new_len) = 0;
	virtual void insert(usize pos, char const* data, usize len) = 0;
	virtual void insert_newline(usize pos) = 0;

	template <usize N>
	VEG_INLINE void append_literal(char const (&data)[N]) {
		insert(size(), &data[0], N - 1);
	}
	VEG_INLINE void append_ln() { insert_newline(size()); }

	VEG_NODISCARD virtual auto data() const VEG_ALWAYS_NOEXCEPT -> char* = 0;
	VEG_NODISCARD virtual auto size() const VEG_ALWAYS_NOEXCEPT -> usize = 0;

protected:
	~Buffer() = default;
};

using BufferMut = Buffer&;
} // namespace fmt
namespace internal {
namespace fmt_ {

struct DbgStructScope {
	DbgStructScope(DbgStructScope const&) = delete;
	DbgStructScope(DbgStructScope&&) = delete;
	auto operator=(DbgStructScope const&) -> DbgStructScope& = delete;
	auto operator=(DbgStructScope&&) -> DbgStructScope& = delete;

	fmt::BufferMut out;
	DbgStructScope(fmt::BufferMut out_) : out(VEG_FWD(out_)) {
		out.append_literal("{");
		++out.indent_level;
	}
	~DbgStructScope() {
		--out.indent_level;
		out.append_ln();
		out.append_literal("}");
	}
};

using BufferMut = veg::fmt::BufferMut;

inline void to_string_impl(BufferMut out, unsigned type, void* arg) {
	usize n = abi::internal::snprintf1(nullptr, 0, type, arg) + 1;

	usize old_size = out.size();
	out.resize(out.size() + n);
	abi::internal::snprintf1(out.data() + old_size, usize(n), type, arg);
	out.resize(old_size + n - 1);
}

struct dbg_i {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		auto _ = static_cast<long long signed>(arg.get());
		fmt_::to_string_impl(VEG_FWD(out), 0U, &_);
	}
};
struct dbg_u {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		auto _ = static_cast<long long unsigned>(arg.get());
		fmt_::to_string_impl(VEG_FWD(out), 1U, &_);
	}
};
struct dbg_p {
	template <typename T>
	static void to_string(BufferMut out, Ref<T*> arg) {
		auto* _ = const_cast<void*>(static_cast<void const volatile*>(arg.get()));
		fmt_::to_string_impl(VEG_FWD(out), 2U, _);
	}
};
struct dbg_pf {
	template <typename Ret, typename... Args>
	static void to_string(BufferMut out, Ref<Ret (*)(Args...)> arg) {
		auto* _ = reinterpret_cast<void*>(arg.get());
		fmt_::to_string_impl(VEG_FWD(out), 2U, _);
	}
};
struct dbg_f {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		auto _ = static_cast<long double>(arg.get());
		fmt_::to_string_impl(VEG_FWD(out), 3U + 4U * sizeof(T), &_);
	}
};
struct dbg_b {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		out.insert(out.size(), arg.get() ? "true" : "false", arg.get() ? 4 : 5);
	}
};

template <bool>
struct dbg_g_impl {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> /*arg*/) {
		out.insert(out.size(), "{?}", 3);
	}
};

template <>
struct dbg_g_impl<true> {

	template <typename T>
	using Access = typename internal::member_extract_access<T>::Type;

	template <typename T, typename... Bases, usize... Is, typename... Members>
	static void to_string_impl(
			BufferMut out,
			Ref<T> arg,
			SimpleITuple<meta_::integer_sequence<usize, Is...>, Members Bases::*...>
					member_ptrs) {

		out.insert(
				out.size(), Access<T>::class_name_ptr, Access<T>::class_name_len);
		out.append_literal(" ");
		{
			fmt_::DbgStructScope _{VEG_FWD(out)};
			VEG_EVAL_ALL(
					(void(_.out.append_ln()),
			     void(_.out.insert(
							 _.out.size(),
							 Access<T>::member_name_ptrs[Is],
							 Access<T>::member_name_lens[Is])),
			     void(_.out.append_literal(": ")),
			     void(fmt::Debug<Members>::to_string(
							 VEG_FWD(_.out),
							 ref(arg.get().*
			             (static_cast<SimpleLeaf<Is, Members Bases::*> const&>(
												member_ptrs)
			                  .inner)))),
			     void(_.out.append_literal(","))

			         ));
		}
	}

	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		to_string_impl(VEG_FWD(out), arg, Access<T>::member_pointers());
	}
};

struct dbg_g {
	template <typename T>
	static void to_string(BufferMut out, Ref<T> arg) {
		dbg_g_impl<internal::member_extract_access<T>::value>::to_string(
				VEG_FWD(out), arg);
	}
};

template <typename T>
using choose_dbg = meta::conditional_t<
		VEG_CONCEPT(same<T, bool>),
		internal::fmt_::dbg_b,
		meta::conditional_t<
				VEG_CONCEPT(signed_integral<T>),
				internal::fmt_::dbg_i,
				meta::conditional_t<
						VEG_CONCEPT(unsigned_integral<T>),
						internal::fmt_::dbg_u,
						meta::conditional_t<
								VEG_CONCEPT(floating_point<T>),
								internal::fmt_::dbg_f,
								meta::conditional_t<
										VEG_CONCEPT(constructible<bool, T>),
										internal::fmt_::dbg_b,
										internal::fmt_::dbg_g>>>>>;

} // namespace fmt_
} // namespace internal

namespace fmt {

template <typename T>
struct Debug : internal::fmt_::choose_dbg<T> {};
template <typename T>
struct Debug<T*> : internal::fmt_::dbg_p {};
template <>
struct Debug<decltype(nullptr)> : internal::fmt_::dbg_p {};
template <typename Ret, typename... Args>
struct Debug<Ret (*)(Args...)> : internal::fmt_::dbg_pf {};

namespace nb {
struct dbg_to {
	template <typename T>
	VEG_INLINE void operator()(BufferMut out, Ref<T> x) const
			VEG_NOEXCEPT_IF(false) {
		Debug<T>::to_string(VEG_FWD(out), x);
	}
};
} // namespace nb
VEG_NIEBLOID(dbg_to);
} // namespace fmt

namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
struct String final : fmt::Buffer {
	struct layout {
		char* ptr;
		usize len;
		usize cap;
	} self = {};

	~String();
	String() = default;
	VEG_INLINE String(String&& other) VEG_ALWAYS_NOEXCEPT : self{other.self} {
		other.self = layout{nullptr, 0, 0};
	}
	String(String const&) = delete;
	auto operator=(String const&) -> String& = delete;
	auto operator=(String&&) -> String& = delete;

	void resize(usize new_len) override;
	void reserve(usize new_cap) override;
	void insert(usize pos, char const* data, usize len) override;
	void insert_newline(usize pos) override;
	void eprint() const VEG_ALWAYS_NOEXCEPT;

	VEG_NODISCARD VEG_INLINE auto data() const VEG_ALWAYS_NOEXCEPT
			-> char* override {
		return self.ptr;
	}
	VEG_NODISCARD VEG_INLINE auto size() const VEG_ALWAYS_NOEXCEPT
			-> usize override {
		return self.len;
	}
};
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace nb {
struct dbg {
	template <typename T>
	auto operator()( //
			T arg,
			unsigned line =
#if VEG_HAS_BUILTIN(__builtin_LINE)
					__builtin_LINE(),
#else
					0,
#endif

			char const* file =
#if VEG_HAS_BUILTIN(__builtin_FILE)
					__builtin_FILE(),
#else
					nullptr,
#endif

			char const* fn =
#if VEG_HAS_BUILTIN(__builtin_FUNCTION)
					__builtin_FUNCTION()
#else
					nullptr
#endif

	) const -> T {
		abi::internal::String out;

		if (line != 0 && file != nullptr && fn != nullptr) {
			auto file_len = usize(std::strlen(file));
			auto fn_len = usize(std::strlen(fn));

			out.append_literal("[");
			out.insert(1, fn, fn_len);
			out.append_literal(":");
			out.insert(1 + fn_len + 1, file, file_len);
			out.append_literal(":");
			fmt::Debug<unsigned>::to_string(out, nb::ref{}(line));
			out.append_literal("] ");
		}
		fmt::Debug<T>::to_string(out, nb::ref{}(arg));
		out.eprint();
		return T(VEG_FWD(arg));
	}
};
} // namespace nb
VEG_NIEBLOID(dbg);
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_FMT_HPP_GQU8XFRUS */
