#ifndef VEG_FN_VIEW_HPP_2EMSNQIXS
#define VEG_FN_VIEW_HPP_2EMSNQIXS

#include "veg/ref.hpp"
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
		return {vptr(&reinterpret_cast<char_ref>(arg))};
	}

	template <typename Fn, typename Ret, typename... Args>
	VEG_INLINE static auto call(StatePtr state, Args... args) noexcept(
			VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Args&&...>)) -> Ret {
		return static_cast<Fn&&>(*static_cast<meta::uncvref_t<Fn>*>(state.ptr))(
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
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Args&&...>)) -> Ret {
		return reinterpret_cast<decltype(+VEG_DECLVAL(Fn&))>(state.fn)(
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
	RawParts _self = {};

	constexpr function_ref_impl() = default;
	template <typename Fn>
	VEG_INLINE function_ref_impl(
			FromRawParts /*unused*/, FromRawParts /*unused*/, Fn&& fn) VEG_NOEXCEPT
			: _self{
						fnref::fn_view_impl<
								fnref::fn_kind<meta::decay_t<Fn>>::value>::address(fn),
						fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
								template call<Fn, Ret, Args...>,
				} {
		assert_non_null<meta::uncvref_t<Fn>>::apply(_self.state);
	}

	VEG_INLINE
	function_ref_impl(FromRawParts /*unused*/, RawParts parts) VEG_NOEXCEPT
			: _self{parts} {}

	VEG_INLINE
	auto _call_fn(Args... args) const VEG_NOEXCEPT_IF(No_Except) -> Ret {
		auto* fn_ptr = reinterpret_cast<void (*)()>(_self.fn_ptr);
		(void)fn_ptr;
		meta::unreachable_if(fn_ptr == nullptr);
		return _self.fn_ptr(_self.state, VEG_FWD(args)...);
	}

	template <fnref::fn_kind_e>
	friend struct fnref::fn_view_impl;
};

} // namespace fnref
} // namespace internal

namespace fn {
template <typename T>
struct FnRef;
template <typename T>
struct FnRefMut;

template <typename T>
struct NothrowFnRef;
template <typename T>
struct NothrowFnRefMut;

template <typename Ret, typename... Args>
struct FnRef<Ret(Args...)>
		: internal::fnref::function_ref_impl<false, Ret, Args...> {
	using Base = internal::fnref::function_ref_impl<false, Ret, Args...>;

	FnRef() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(fn<Fn, Ret, Args&&...>)),
			VEG_INLINE FnRef, /* NOLINT */
			(fn, Ref<Fn>))
	VEG_NOEXCEPT : Base{
										 from_raw_parts,
										 from_raw_parts,
										 fn.get(),
								 } {}

	VEG_INLINE
	FnRef(NothrowFnRef<Ret(Args...)> fn) VEG_NOEXCEPT
			: Base{
						from_raw_parts,
						{fn._self.state, fn._self.fn_ptr},
				} {}

	VEG_INLINE
	auto operator()(Args... args) const -> Ret {
		return this->_call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct FnRefMut<Ret(Args...)>
		: internal::fnref::function_ref_impl<false, Ret, Args...> {
	using Base = internal::fnref::function_ref_impl<false, Ret, Args...>;

	FnRefMut() = delete;

	VEG_NO_COPY(FnRefMut);

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, Args&&...>)),
			VEG_INLINE FnRefMut, /* NOLINT */
			(fn, RefMut<Fn>))
	VEG_NOEXCEPT : Base{
										 from_raw_parts,
										 from_raw_parts,
										 fn.get(),
								 } {}

	VEG_INLINE
	FnRefMut(NothrowFnRefMut<Ret(Args...)> fn) VEG_NOEXCEPT
			: Base{
						from_raw_parts,
						{fn._self.state, fn._self.fn_ptr},
				} {}
	VEG_INLINE
	FnRefMut(NothrowFnRef<Ret(Args...)> fn) VEG_NOEXCEPT
			: Base{
						from_raw_parts,
						{fn._self.state, fn._self.fn_ptr},
				} {}
	VEG_INLINE
	FnRefMut(FnRef<Ret(Args...)> fn) VEG_NOEXCEPT
			: Base{
						from_raw_parts,
						{fn._self.state, fn._self.fn_ptr},
				} {}

	VEG_INLINE
	auto operator()(Args... args) -> Ret {
		return this->_call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct NothrowFnRef<Ret(Args...)>
		: internal::fnref::function_ref_impl<true, Ret, Args...> {
	using Base = internal::fnref::function_ref_impl<true, Ret, Args...>;

	NothrowFnRef() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(nothrow_fn<Fn, Ret, Args&&...>)),
			VEG_INLINE NothrowFnRef, /* NOLINT */
			(fn, Ref<Fn>))
	VEG_NOEXCEPT : Base{
										 from_raw_parts,
										 from_raw_parts,
										 fn.get(),
								 } {}

	VEG_INLINE
	auto operator()(Args... args) const VEG_NOEXCEPT -> Ret {
		return this->_call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct NothrowFnRefMut<Ret(Args...)>
		: internal::fnref::function_ref_impl<true, Ret, Args...> {
	using Base = internal::fnref::function_ref_impl<true, Ret, Args...>;

	NothrowFnRefMut() = delete;

	VEG_NO_COPY(NothrowFnRefMut);

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, Args&&...>)),
			VEG_INLINE NothrowFnRefMut, /* NOLINT */
			(fn, RefMut<Fn>))
	VEG_NOEXCEPT : Base{
										 from_raw_parts,
										 from_raw_parts,
										 fn.get(),
								 } {}

	VEG_INLINE
	NothrowFnRefMut(NothrowFnRef<Ret(Args...)> fn) VEG_NOEXCEPT
			: Base{
						from_raw_parts,
						{fn._self.state, fn._self.fn_ptr},
				} {}

	VEG_INLINE
	auto operator()(Args... args) VEG_NOEXCEPT -> Ret {
		return this->_call_fn(VEG_FWD(args)...);
	}
};
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"

#endif /* end of include guard VEG_FN_VIEW_HPP_2EMSNQIXS */
