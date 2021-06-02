#ifndef VEG_FN_VIEW_HPP_2EMSNQIXS
#define VEG_FN_VIEW_HPP_2EMSNQIXS

#include "veg/util/assert.hpp"
#include "veg/util/unreachable.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace fnref {

union StatePtr {
	void* ptr;
	void (*fn)();
};

template <bool No_Except, typename Ret, typename... Args>
struct RawParts {
	StatePtr state;
	auto (*fn_ptr)(StatePtr, Args...) -> Ret;
};

template <typename Ret, typename... Args>
struct RawParts<true, Ret, Args...> {
	StatePtr state;
	auto (*fn_ptr)(StatePtr, Args...) noexcept -> Ret;
};

template <typename Ret>
struct discard_void {
	// non void case
	template <typename Fn, typename... Args>
	VEG_INLINE static auto apply(Fn&& fn, Args&&... args)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
		return VEG_FWD(fn)(VEG_FWD(args)...);
	}
};
template <>
struct discard_void<void> {
	template <typename Fn, typename... Args>
	VEG_INLINE static auto apply(Fn&& fn, Args&&... args)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> void {
		static_cast<void>(VEG_FWD(fn)(VEG_FWD(args)...));
	}
};

enum struct fn_kind_e {
	fn_ptr,
	fn_obj,
};

template <typename T>
struct fn_kind : meta::constant<fn_kind_e, fn_kind_e::fn_obj> {};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)>
		: meta::constant<fn_kind_e, fn_kind_e::fn_ptr> {};

VEG_CPP17(
template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept>
    : meta::constant<fn_kind_e, fn_kind_e::fn_ptr> {};
)

template <fn_kind_e Kind>
struct fn_view_impl;

template <>
struct fn_view_impl<fn_kind_e::fn_obj> {
	template <typename Fn>
	VEG_INLINE static auto address(Fn& arg) VEG_NOEXCEPT -> StatePtr {
		using char_ref = char const volatile&;
		using vptr = void*;
		using ptr = Fn*;
		return {ptr(vptr(&reinterpret_cast<char_ref>(arg)))};
	}

	template <typename Fn, typename Ret, typename... Args>
	VEG_INLINE static auto call(StatePtr state, Args... args)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
		return discard_void<Ret>::apply(
				static_cast<Fn&&>(*static_cast<meta::uncvref_t<Fn>*>(state.ptr)),
				VEG_FWD(args)...);
	}
};
template <>
struct fn_view_impl<fn_kind_e::fn_ptr> {
	template <typename Fn>
	VEG_INLINE static auto address(Fn& arg) VEG_NOEXCEPT -> StatePtr {
		StatePtr rv; // NOLINT
		rv.fn = reinterpret_cast<void (*)()>(+arg);
		return rv;
	}

	template <typename Fn, typename Ret, typename... Args>
	VEG_INLINE static auto call(StatePtr state, Args... args)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
		return discard_void<Ret>::apply(
				reinterpret_cast<decltype(+VEG_DECLVAL(Fn&))>(state.fn),
				VEG_FWD(args)...);
	}
};

struct assert_non_null_ref {
	VEG_INLINE
	static void apply(StatePtr /*arg*/) VEG_NOEXCEPT {}
};
struct assert_non_null_ptr {
	VEG_INLINE
	static void apply(StatePtr arg) VEG_NOEXCEPT {
		VEG_INTERNAL_ASSERT_PRECONDITION(bool(arg.fn != nullptr));
	}
};
template <typename T>
struct assert_non_null : assert_non_null_ref {};
template <typename Ret, typename... Args>
struct assert_non_null<Ret (*)(Args...)> : assert_non_null_ptr {};

template <bool No_Except, typename Ret, typename... Args>
struct function_ref_impl {
	using RawParts = fnref::RawParts<No_Except, Ret, Args...>;
	RawParts self = {};

	constexpr function_ref_impl() = default;
	template <typename Fn>
	VEG_INLINE
	function_ref_impl(char /*unused*/, char /*unused*/, Fn&& fn) VEG_NOEXCEPT
			: self{
						fnref::fn_view_impl<
								fnref::fn_kind<meta::decay_t<Fn>>::value>::address(fn),
						fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
								template call<Fn, Ret, Args...>,
				} {
		assert_non_null<meta::uncvref_t<Fn>>::apply(self.state);
	}

	VEG_INLINE
	function_ref_impl(char /*unused*/, RawParts parts) VEG_NOEXCEPT
			: self{parts} {}

	VEG_INLINE
	auto call_fn(Args... args) const VEG_NOEXCEPT_IF(No_Except) -> Ret {
		auto* fn_ptr = reinterpret_cast<void (*)()>(self.fn_ptr);
		(void)fn_ptr;
		meta::unreachable_if(fn_ptr == nullptr);
		return self.fn_ptr(self.state, VEG_FWD(args)...);
	}

	template <fnref::fn_kind_e>
	friend struct fnref::fn_view_impl;
};

} // namespace fnref
} // namespace internal

namespace fn {
template <typename T>
struct FnView;
template <typename T>
struct FnOnceView;

template <typename Ret, typename... Args>
struct FnView<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	template <typename T>
	friend struct FnView;
	template <typename T>
	friend struct FnOnceView;

public:
	FnView() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable_r<Fn&, Ret, Args&&...>)),
			VEG_INLINE FnView, /* NOLINT */
			(/*tag*/, AsRef),
			(fn, Fn&&))
	VEG_NOEXCEPT : base({}, {}, fn) {}

#if __cplusplus >= 201703L
	VEG_INLINE
	FnView(FnView<Ret(Args...) noexcept> fn) VEG_NOEXCEPT
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}
#endif

	VEG_INLINE
	auto operator()(Args... args) const -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct FnOnceView<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...>,
			internal::NoCopyCtor,
			internal::NoCopyAssign {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	template <typename T>
	friend struct FnView;
	template <typename T>
	friend struct FnOnceView;

public:
	FnOnceView() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable_r<Fn, Ret, Args&&...>)),
			VEG_INLINE FnOnceView, /* NOLINT */
			(/*tag*/, AsRef),
			(fn, Fn&&))
	VEG_NOEXCEPT : base({}, {}, VEG_FWD(fn)) {}

#if __cplusplus >= 201703L
	VEG_INLINE
	FnOnceView(FnView<Ret(Args...) noexcept> fn) VEG_NOEXCEPT
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}
#endif

	VEG_INLINE
	FnOnceView(FnView<Ret(Args...)> fn) VEG_NOEXCEPT
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	VEG_INLINE
	auto operator()(Args... args) const&& -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

#if __cplusplus >= 201703L
template <typename Ret, typename... Args>
struct FnView<Ret(Args...) noexcept>
		: private internal::fnref::function_ref_impl<true, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	template <typename T>
	friend struct FnView;
	template <typename T>
	friend struct FnOnceView;

public:
	FnView() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(nothrow_invocable_r<Fn&, Ret, Args&&...>)),
			VEG_INLINE FnView, /* NOLINT */
			(/*tag*/, AsRef),
			(fn, Fn&&))
	VEG_NOEXCEPT : base({}, {}, fn) {}

	VEG_INLINE
	auto operator()(Args... args) const VEG_NOEXCEPT -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct FnOnceView<Ret(Args...) noexcept>
		: private internal::fnref::function_ref_impl<true, Ret, Args...>,
			internal::NoCopyCtor,
			internal::NoCopyAssign {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	template <typename T>
	friend struct FnView;
	template <typename T>
	friend struct FnOnceView;

public:
	FnOnceView() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(nothrow_invocable_r<Fn, Ret, Args&&...>)),
			VEG_INLINE FnOnceView, /* NOLINT */
			(/*tag*/, AsRef),
			(fn, Fn&&))
	VEG_NOEXCEPT : base({}, {}, VEG_FWD(fn)) {}

	VEG_INLINE
	FnOnceView(FnView<Ret(Args...) noexcept> fn) VEG_NOEXCEPT
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	VEG_INLINE
	auto operator()(Args... args) const && VEG_NOEXCEPT -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};
#endif

} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"

#endif /* end of include guard VEG_FN_VIEW_HPP_2EMSNQIXS */
