#ifndef __VEG_OPTION_HPP_8NVLXES2S
#define __VEG_OPTION_HPP_8NVLXES2S

#include "veg/assert.hpp"
#include "veg/internal/memory.hpp"
#include "veg/internal/storage.hpp"
#include "veg/internal/cmp.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace meta {
template <typename T>
struct is_mostly_trivial<option<T>> : is_mostly_trivial<T> {};
template <typename T>
struct is_trivially_relocatable<option<T>> : is_trivially_relocatable<T> {};
template <typename T>
struct is_option : false_type {};
template <typename T>
struct is_option<option<T>> : true_type {};

template <typename T>
struct option_inner {};
template <typename T>
struct option_inner<option<T>> {
	using type = T;
};
} // namespace meta
struct some_ref_t {
	template <typename T>
	VEG_CPP14(constexpr)
	auto operator()(T&& arg) const noexcept -> option<T&&> {
		return {some, VEG_FWD(arg)};
	}

private:
	constexpr some_ref_t() = default;
	constexpr explicit some_ref_t(some_ref_t* /*unused*/) noexcept {}
	template <typename T>
	friend struct meta::internal::static_const;
};
VEG_INLINE_VAR(some_ref, some_ref_t);

namespace internal {
namespace option_ {
struct hidden_tag {};

template <bool NoExcept, typename T, typename O>
VEG_CPP14(constexpr)
void copy_assign(O& lhs, O const& rhs) noexcept(NoExcept) {
	if (rhs.is_engaged()) {
		if (lhs.is_engaged()) {
			lhs.assign(rhs._get());
		} else {
			lhs._emplace(storage::copy_ctor_fn<T>{rhs._get()});
		}
	} else {
		if (lhs.is_engaged()) {
			lhs.destroy();
		}
		lhs.engaged = rhs.engaged;
	}
}

template <bool NoExcept, typename T, typename O>
VEG_CPP14(constexpr)
void move_assign(O& lhs, O&& rhs) noexcept(NoExcept) {
	if (rhs.is_engaged()) {
		if (lhs.is_engaged()) {
			lhs.assign(VEG_FWD(rhs)._get());
		} else {
			lhs._emplace(storage::move_ctor_fn<T>{VEG_FWD(rhs)._get()});
		}
	} else {
		if (lhs.is_engaged()) {
			lhs.destroy();
		}
		lhs.engaged = rhs.engaged;
	}
}

enum kind { has_sentinel, trivial, non_trivial };

template <typename T, bool = (meta::value_sentinel_for<T>::value > 1)>
struct value_sentinel_impl
		: std::integral_constant<i64, i64(static_cast<unsigned char>(-3))> {
	static VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
		if (i <= static_cast<unsigned char>(-3)) {
			option<T> val = none;
			val.engaged = static_cast<unsigned char>(2 + i);
			return val;
		}
		abi::internal::terminate();
	}
	static VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
		return (arg.engaged < 2) ? -1 : (arg.engaged - 2);
	}
};

template <typename T>
struct value_sentinel_impl<T, true>
		: std::integral_constant<i64, meta::value_sentinel_for<T>::value - 1> {
	static VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
		return option<T>{meta::value_sentinel_for<T>::invalid(i + 1)};
	}
	static VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
		if (arg) {
			return -1;
		}
		return meta::value_sentinel_for<T>::id(
				arg.unwrap_unchecked_invalid(unsafe));
	}
};

template <typename Fn>
struct do_if {
	bool cond;
	Fn fn;
	VEG_CPP20(constexpr) void operator()() const noexcept {
		if (cond) {
			fn();
		}
	}
};

struct make_none { // NOLINT
	unsigned char* c;
	VEG_CPP20(constexpr) void operator()() const noexcept { construct_at(c); }
};

template <typename T>
using kind_v = std::integral_constant<
		kind,
		((meta::value_sentinel_for<storage::storage<T>>::value > 0) //
         ? has_sentinel
         : ((VEG_CONCEPT(mostly_trivial<T>) &&     //
             VEG_CONCEPT(copy_assignable<T>) &&    //
             VEG_CONCEPT(move_assignable<T>) &&    //
             VEG_CONCEPT(constructible<T>) &&      //
             VEG_CONCEPT(copy_constructible<T>) && //
             VEG_CONCEPT(move_constructible<T>)))
               ? trivial
               : non_trivial)>;

template <typename T, kind = kind_v<T>::value>
struct option_storage_base;

template <typename T>
struct option_storage_base<T, trivial> {
	static_assert(
			VEG_CONCEPT(trivially_copyable<T>),
			"mostly_trivial is probably mis-defined");

	storage::storage<T> some = {};
	unsigned char engaged = 0;

	VEG_CPP14(constexpr) option_storage_base() = default;
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit option_storage_base(
			T&& val) noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			: some(storage::hidden_tag2{}, VEG_FWD(val)), engaged(1) {}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit option_storage_base(
			hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>))
			: some(storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...),
				engaged(1) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return engaged == 1;
	}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _emplace(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());
		some = storage::storage<T>(
				storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...);
		engaged = 1;
	}

	template <typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _arg_emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());
		some = storage::storage<T>(storage::hidden_tag0{}, VEG_FWD(args)...);
		engaged = 1;
	}

	template <typename U>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
			noexcept(some.get_mut() = VEG_FWD(rhs))) {
		VEG_INTERNAL_ASSERT(is_engaged());
		some.get_mut() = VEG_FWD(rhs);
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		engaged = 0;
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some._get();
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() & noexcept
			-> T& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some.get_mut();
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return VEG_FWD(some).get_mov_ref();
	}
};

template <typename T>
struct option_storage_base<T, has_sentinel> {
	using sentinel_traits = meta::value_sentinel_for<storage::storage<T>>;

	static_assert(VEG_CONCEPT(trivially_destructible<T>), "um");
	static_assert(VEG_CONCEPT(trivially_copyable<storage::storage<T>>), "err");
	static_assert(VEG_CONCEPT(move_constructible<storage::storage<T>>), "err");
	static_assert(VEG_CONCEPT(move_assignable<storage::storage<T>>), "err");

	storage::storage<T> some = sentinel_traits::invalid(0);

	VEG_CPP14(constexpr) option_storage_base() = default;
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit option_storage_base(
			T&& val) noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			: some(storage::hidden_tag2{}, VEG_FWD(val)) {}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit option_storage_base(
			hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>))
			: some(storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return sentinel_traits::id(some) < 0;
	}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _emplace(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());
		some = storage::storage<T>(
				storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...);
	}

	template <typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _arg_emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());
		some = storage::storage<T>(storage::hidden_tag0{}, VEG_FWD(args)...);
	}

	template <typename U>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
			noexcept(some.get_mut() = VEG_FWD(rhs))) {
		VEG_INTERNAL_ASSERT(is_engaged());
		some.get_mut() = VEG_FWD(rhs);
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void destroy() noexcept {
		some = sentinel_traits::invalid(0);
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(
			constexpr) auto unwrap_unchecked_invalid(unsafe_t /*tag*/) const& noexcept
			-> T const& {
		return some._get();
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some._get();
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() & noexcept
			-> T& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some.get_mut();
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return VEG_FWD(some).get_mov_ref();
	}
};

template <bool Trivial>
struct option_storage_base_option_emplace {
	template <typename T, typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) static void apply(
			T& self,
			Fn&& fn,
			Args&&... args) noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>)) {
		VEG_INTERNAL_ASSERT(!self.is_engaged());
		self.some = invoke(VEG_FWD(fn), VEG_FWD(args)...);
	}

	template <typename Inner, typename T, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) static void apply2(T& self, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<Inner, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!self.is_engaged());
		self.some = Inner(VEG_FWD(args)...);
	}
};

template <>
struct option_storage_base_option_emplace<false> {
	template <typename T, typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) static void apply(
			T& self,
			Fn&& fn,
			Args&&... args) noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>)) {
		VEG_INTERNAL_ASSERT(!self.is_engaged());
		// destroy_at(addressof(self.some)); // no-op
		construct_with(addressof(self.some), VEG_FWD(fn), VEG_FWD(args)...);
	}

	template <typename Inner, typename T, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) static void apply2(T& self, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<Inner, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!self.is_engaged());
		construct_at(addressof(self.some), VEG_FWD(args)...);
	}
};

template <typename T>
struct option_storage_base<option<T>, has_sentinel> {
	using sentinel_traits = meta::value_sentinel_for<option<T>>;
	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _emplace(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) {
		option_storage_base_option_emplace<VEG_CONCEPT(mostly_trivial<T>)>::apply(
				*this, VEG_FWD(fn), VEG_FWD(args)...);
	}

	template <typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void _arg_emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {
		option_storage_base_option_emplace<VEG_CONCEPT(
				mostly_trivial<T>)>::template apply2<T>(*this, VEG_FWD(args)...);
	}

	option<T> some = sentinel_traits::invalid(0);

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_storage_base() = default;
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr)
	explicit option_storage_base(option<T>&& val) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>))
			: some(VEG_FWD(val)) {}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit option_storage_base(
			hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>))
			: some(invoke(VEG_FWD(fn), VEG_FWD(args)...)) {}

	VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return sentinel_traits::id(some) < 0;
	}

	template <typename U>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
			noexcept(some = VEG_FWD(rhs))) {
		VEG_INTERNAL_ASSERT(is_engaged());
		some = VEG_FWD(rhs);
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void destroy() noexcept {
		some = sentinel_traits::invalid(0);
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(
			constexpr) auto unwrap_unchecked_invalid(unsafe_t /*tag*/) const& noexcept
			-> option<T> const& {
		return some;
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() const& noexcept -> option<T> const& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() & noexcept
			-> option<T>& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto _get() && noexcept -> option<T>&& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return VEG_FWD(some);
	}
};

template <typename T, bool = VEG_CONCEPT(trivially_destructible<T>)>
struct option_storage_nontrivial_base {
	union {
		unsigned char none = {};
		storage::storage<T> some;
	};
	unsigned char engaged = 0;

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_storage_nontrivial_base() noexcept {};
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_storage_nontrivial_base(
			i64 /* unused */, unsigned char id) noexcept
			: engaged{id} {};

	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr)
	explicit option_storage_nontrivial_base(T&& val) //
			noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			: some(storage::hidden_tag2{}, VEG_FWD(val)), engaged(1) {}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) explicit option_storage_nontrivial_base(
			hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>))
			: some(storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...),
				engaged(1) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return engaged == 1;
	}
	VEG_CPP14(constexpr)
	HEDLEY_ALWAYS_INLINE void set_engaged(bool b) noexcept {
		engaged = static_cast<unsigned char>(b);
	}
};

template <typename T>
struct
		option_storage_nontrivial_base // NOLINT(cppcoreguidelines-special-member-functions)
		<T, false> {
	union {
		unsigned char none = {};
		storage::storage<T> some;
	};
	unsigned char engaged = 0;
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_storage_nontrivial_base() noexcept {};
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_storage_nontrivial_base(
			i64 /* unused */, unsigned char id) noexcept
			: engaged{id} {};
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr)
	explicit option_storage_nontrivial_base(T&& val) //
			noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			: some(storage::hidden_tag2{}, VEG_FWD(val)), engaged(1) {}

	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) explicit option_storage_nontrivial_base(
			hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>))
			: some(storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...),
				engaged(1) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return engaged == 1;
	}
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void set_engaged(bool b) noexcept {
		engaged = static_cast<unsigned char>(b);
	}

	HEDLEY_ALWAYS_INLINE
	VEG_CPP20(constexpr) ~option_storage_nontrivial_base() {
		if (engaged == 1) {
			destroy_at(addressof(some));
		} else {
			destroy_at(addressof(none));
		}
	}
};

template <typename T>
struct option_storage_base<T, non_trivial> : option_storage_nontrivial_base<T> {

	using option_storage_nontrivial_base<T>::option_storage_nontrivial_base;
	using option_storage_nontrivial_base<T>::some;
	using option_storage_nontrivial_base<T>::none;
	using option_storage_nontrivial_base<T>::is_engaged;
	using option_storage_nontrivial_base<T>::set_engaged;

	template <typename U>
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
			noexcept(some.get_mut() = VEG_FWD(rhs))) {
		VEG_INTERNAL_ASSERT(is_engaged());
		some.get_mut() = VEG_FWD(rhs);
	}

	template <typename Fn, typename... Args>
	VEG_CPP20(constexpr)
	HEDLEY_ALWAYS_INLINE void _emplace(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());

		defer<do_if<make_none>> guard{{true, {addressof(none)}}};

		construct_at(
				addressof(some), storage::hidden_tag1{}, VEG_FWD(fn), VEG_FWD(args)...);

		guard.fn.cond = false;
		set_engaged(true);
	}

	template <typename... Args>
	HEDLEY_ALWAYS_INLINE
	VEG_CPP20(constexpr) void _arg_emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {
		VEG_INTERNAL_ASSERT(!is_engaged());

		defer<do_if<make_none>> guard{{true, {addressof(none)}}};

		construct_at(addressof(some), storage::hidden_tag0{}, VEG_FWD(args)...);

		guard.fn.cond = false;
		set_engaged(true);
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		set_engaged(false);
		defer<make_none> guard{{addressof(none)}};
		destroy_at(addressof(some));
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() const& noexcept
			-> T const& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some._get();
	}
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return some.get_mut();
	}
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
		VEG_INTERNAL_ASSERT(is_engaged());
		return VEG_FWD(some).get_mov_ref();
	}
};

template <
		typename T,
		bool = (kind_v<T>::value != non_trivial) ||
					 VEG_CONCEPT(trivially_copy_constructible<T>) ||
					 !VEG_CONCEPT(copy_constructible<T>)>
struct option_copy_ctor_base : option_storage_base<T> {
	using option_storage_base<T>::option_storage_base;
};
template <typename T>
struct option_copy_ctor_base<T, false> : option_storage_base<T> {
	using option_storage_base<T>::option_storage_base;

	~option_copy_ctor_base() = default;
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_copy_ctor_base(option_copy_ctor_base const& rhs) noexcept(
					VEG_CONCEPT(nothrow_copy_constructible<T>))
			: option_storage_base<T>{0, rhs.engaged} {
		if (rhs.is_engaged()) {
			this->engaged = 0;
			this->_arg_emplace(rhs._get());
		}
	}
	option_copy_ctor_base /* NOLINT */ (option_copy_ctor_base&&) = default;
	auto operator=(option_copy_ctor_base const&)
			-> option_copy_ctor_base& = default;
	auto operator= /* NOLINT */(option_copy_ctor_base&&)
			-> option_copy_ctor_base& = default;
};

template <
		typename T,
		bool = (kind_v<T>::value != non_trivial) ||
					 VEG_CONCEPT(trivially_move_constructible<T>) ||
					 !VEG_CONCEPT(constructible<T, T&&>)>
struct option_move_ctor_base : option_copy_ctor_base<T> {
	using option_copy_ctor_base<T>::option_copy_ctor_base;
};

template <typename T>
struct option_move_ctor_base<T, false> : option_copy_ctor_base<T> {
	using option_copy_ctor_base<T>::option_copy_ctor_base;

	~option_move_ctor_base() = default;
	option_move_ctor_base(option_move_ctor_base const&) = default;
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_move_ctor_base(option_move_ctor_base&& rhs) noexcept(
					VEG_CONCEPT(nothrow_move_constructible<T>))
			: option_copy_ctor_base<T>{0, rhs.engaged} {
		if (rhs.is_engaged()) {
			this->engaged = 0;
			this->_arg_emplace(VEG_FWD(rhs)._get());
		}
	}
	auto operator=(option_move_ctor_base const&)
			-> option_move_ctor_base& = default;
	auto operator= /* NOLINT */(option_move_ctor_base&&)
			-> option_move_ctor_base& = default;
};

template <
		typename T,
		bool = (kind_v<T>::value != non_trivial) ||
					 VEG_CONCEPT(trivially_copy_assignable<T>) ||
					 !VEG_CONCEPT(copy_assignable<T>)>
struct option_copy_assign_base : option_move_ctor_base<T> {
	using option_move_ctor_base<T>::option_move_ctor_base;
};
template <typename T>
struct option_copy_assign_base<T, false> : option_move_ctor_base<T> {
	using option_move_ctor_base<T>::option_move_ctor_base;
	~option_copy_assign_base() = default;

	option_copy_assign_base(option_copy_assign_base const&) = default;
	option_copy_assign_base /* NOLINT */ (option_copy_assign_base&&) = default;
	VEG_CPP14(constexpr)
	auto operator= // NOLINT(cert-oop54-cpp)
			(option_copy_assign_base const& rhs) noexcept(
					(VEG_CONCEPT(nothrow_copy_constructible<T>) &&
	         VEG_CONCEPT(nothrow_copy_assignable<T>)))
					-> option_copy_assign_base& {
		option_::copy_assign<noexcept(*this = rhs), T>(*this, rhs);
		return *this;
	}
	auto operator= /* NOLINT */(option_copy_assign_base&&)
			-> option_copy_assign_base& = default;
};

template <
		typename T,
		bool = (kind_v<T>::value != non_trivial) ||
					 VEG_CONCEPT(trivially_move_assignable<T>) ||
					 !VEG_CONCEPT(move_assignable<T>)>
struct option_move_assign_base : option_copy_assign_base<T> {
	using option_copy_assign_base<T>::option_copy_assign_base;
};
template <typename T>
struct option_move_assign_base<T, false> : option_copy_assign_base<T> {
	using option_copy_assign_base<T>::option_copy_assign_base;

	~option_move_assign_base() = default;
	option_move_assign_base(option_move_assign_base const&) = default;
	option_move_assign_base /* NOLINT */ (option_move_assign_base&&) = default;
	auto operator=(option_move_assign_base const&)
			-> option_move_assign_base& = default;
	VEG_CPP14(constexpr)
	auto operator=(option_move_assign_base&& rhs) noexcept(
			(VEG_CONCEPT(nothrow_move_constructible<T>) &&
	     VEG_CONCEPT(nothrow_move_assignable<T>))) -> option_move_assign_base& {
		option_::move_assign<noexcept(*this = VEG_FWD(rhs)), T>(
				*this, VEG_FWD(rhs));
		return *this;
	}
};

template <typename T>
struct option_sp_mem_base : option_storage_base<T> {
	~option_sp_mem_base() = default;
	using option_storage_base<T>::option_storage_base;

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_sp_mem_base(option_sp_mem_base const& rhs) noexcept(
					VEG_CONCEPT(nothrow_copy_constructible<T>))
			: option_storage_base<T>{0, rhs.engaged} {
		if (rhs.is_engaged()) {
			this->engaged = 0;
			this->_arg_emplace(rhs._get());
		}
	}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_sp_mem_base(option_sp_mem_base&& rhs) noexcept(
					VEG_CONCEPT(nothrow_move_constructible<T>))
			: option_storage_base<T>{0, rhs.engaged} {
		if (rhs.is_engaged()) {
			this->engaged = 0;
			this->_arg_emplace(VEG_FWD(rhs)._get());
		}
	}

	VEG_CPP14(constexpr)
	auto operator= // NOLINT(cert-oop54-cpp)
			(option_sp_mem_base const& rhs) noexcept(
					(VEG_CONCEPT(nothrow_copy_constructible<T>) &&
	         VEG_CONCEPT(nothrow_copy_assignable<T>))) -> option_sp_mem_base& {
		option_::copy_assign<noexcept(*this = rhs), T>(*this, rhs);
		return *this;
	}
	VEG_CPP14(constexpr)
	auto operator=(option_sp_mem_base&& rhs) noexcept(
			(VEG_CONCEPT(nothrow_move_constructible<T>) &&
	     VEG_CONCEPT(nothrow_move_assignable<T>))) -> option_sp_mem_base& {
		option_::move_assign<noexcept(*this = VEG_FWD(rhs)), T>(
				*this, VEG_FWD(rhs));
		return *this;
	}
};

template <typename T>
struct option_move_sp_mem_base : option_storage_base<T> {
	~option_move_sp_mem_base() = default;
	using option_storage_base<T>::option_storage_base;

	option_move_sp_mem_base(option_move_sp_mem_base const&) = delete;
	auto operator=(option_move_sp_mem_base const& rhs)
			-> option_move_sp_mem_base& = delete;

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_move_sp_mem_base(option_move_sp_mem_base&& rhs) noexcept(
					VEG_CONCEPT(nothrow_move_constructible<T>))
			: option_storage_base<T>{0, rhs.engaged} {
		if (rhs.is_engaged()) {
			this->engaged = 0;
			this->_emplace(storage::move_ctor_fn<T>{VEG_FWD(rhs)._get()});
		}
	}

	VEG_CPP14(constexpr)
	auto operator=(option_move_sp_mem_base&& rhs) noexcept(
			(VEG_CONCEPT(nothrow_move_constructible<T>) &&
	     VEG_CONCEPT(nothrow_move_assignable<T>))) -> option_move_sp_mem_base& {
		option_::move_assign<noexcept(*this = VEG_FWD(rhs)), T>(
				*this, VEG_FWD(rhs));
		return *this;
	}
};

template <typename T, bool = VEG_CONCEPT(move_constructible<T>)>
struct option_move_ctor_base2
		: meta::conditional_t<
					(kind_v<T>::value != non_trivial),
					option_storage_base<T>,
					meta::conditional_t<
							((!VEG_CONCEPT(trivially_move_assignable<T>) &&
                !VEG_CONCEPT(trivially_move_constructible<T>) &&
                !VEG_CONCEPT(trivially_copy_assignable<T>) &&
                !VEG_CONCEPT(trivially_copy_constructible<T>)) &&
               (VEG_CONCEPT(move_assignable<T>) &&
                VEG_CONCEPT(move_constructible<T>))),
							meta::conditional_t<
									(VEG_CONCEPT(copy_assignable<T>) &&
                   VEG_CONCEPT(copy_constructible<T>)),
									option_sp_mem_base<T>,
									option_move_sp_mem_base<T>>,
							option_move_assign_base<T>>> {
	using _veg_base = meta::conditional_t<
			(kind_v<T>::value != non_trivial),
			option_storage_base<T>,
			meta::conditional_t<
					(kind_v<T>::value != non_trivial),
					option_storage_base<T>,
					meta::conditional_t<
							((!VEG_CONCEPT(trivially_move_assignable<T>) &&
	              !VEG_CONCEPT(trivially_move_constructible<T>) &&
	              !VEG_CONCEPT(trivially_copy_assignable<T>) &&
	              !VEG_CONCEPT(trivially_copy_constructible<T>)) &&
	             (VEG_CONCEPT(move_assignable<T>) &&
	              VEG_CONCEPT(move_constructible<T>))),
							meta::conditional_t<
									(VEG_CONCEPT(copy_assignable<T>) &&
	                 VEG_CONCEPT(copy_constructible<T>)),
									option_sp_mem_base<T>,
									option_move_sp_mem_base<T>>,
							option_move_assign_base<T>>>>;

	option_move_ctor_base2() = default;
	explicit VEG_CPP14(constexpr) option_move_ctor_base2(T value) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>))
			: _veg_base(VEG_FWD(value)) {}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_move_ctor_base2(some_t /*tag*/, T value) noexcept(
					VEG_CONCEPT(nothrow_move_constructible<T>))
			: _veg_base(VEG_FWD(value)) {}

	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(invocable<Fn, Args...>) &&
					VEG_SAME_AS(T, (meta::invoke_result_t<Fn, Args...>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_move_ctor_base2,

			(/*tag*/, some_t),
			(/*tag*/, inplace_t),
			(fn, Fn&&),
			(... args, Args&&))
	noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>))
			: _veg_base(
						internal::option_::hidden_tag{}, VEG_FWD(fn), VEG_FWD(args)...) {}
};

template <typename T>
struct option_move_ctor_base2<T&, true>
		: option_storage_base<T&, has_sentinel> {
	using _veg_base = option_storage_base<T&, has_sentinel>;

	option_move_ctor_base2() = default;
	explicit VEG_CPP14(constexpr) option_move_ctor_base2(T& value) noexcept
			: _veg_base(VEG_FWD(value)) {}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_move_ctor_base2(some_t /*tag*/, T& value) noexcept
			: _veg_base(VEG_FWD(value)) {}

	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(invocable<Fn, Args...>) &&
					VEG_SAME_AS(T, (meta::invoke_result_t<Fn, Args...>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_move_ctor_base2,

			(/*tag*/, some_t),
			(/*tag*/, inplace_t),
			(fn, Fn&&),
			(... args, Args&&))
	noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>))
			: _veg_base(
						internal::option_::hidden_tag{}, VEG_FWD(fn), VEG_FWD(args)...) {}
};

template <typename T>
struct option_move_ctor_base2<T, false> : option_move_assign_base<T> {
	option_move_ctor_base2() = default;
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(invocable<Fn, Args...>) &&
					VEG_SAME_AS(T, (meta::invoke_result_t<Fn, Args...>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) option_move_ctor_base2,

			(/*tag*/, some_t),
			(/*tag*/, inplace_t),
			(fn, Fn&&),
			(... args, Args&&))
	noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>))
			: internal::option_::option_move_assign_base<T>(
						internal::option_::hidden_tag{}, VEG_FWD(fn), VEG_FWD(args)...) {}
};

template <typename To>
struct into_fn {
	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T&& ref) const
			noexcept(VEG_CONCEPT(nothrow_constructible<To, T&&>)) -> To {
		return VEG_FWD(ref);
	}
};

template <typename T, bool = VEG_CONCEPT(move_constructible<T>)>
struct option_move_interface_base {
	VEG_CPP14(constexpr)
	auto clone() && noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			-> option<T> {
		auto&& self = static_cast<option<T>&&>(*this);
		return VEG_FWD(self)._get().map(into_fn<T>{});
	}

	VEG_NODISCARD
	VEG_CPP14(constexpr)
	auto take() noexcept(VEG_CONCEPT(move_constructible<T>)) -> option<T> {
		auto& self = static_cast<option<T>&>(*this);
		if (self) {
			T val = static_cast<option<T>&&>(self)._get();
			self = none;
			return {some, VEG_FWD(val)};
		}
		return none;
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto unwrap_unchecked(
			unsafe_t /*tag*/) && noexcept(VEG_CONCEPT(nothrow_move_constructible<T>))
			-> T {
		auto&& self = static_cast<option<T>&&>(*this);
		return VEG_FWD(self)._get();
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto unwrap() && noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>)) -> T {
		auto& self = static_cast<option<T>&>(*this);
		self.unwrap_assert();
		return static_cast<option<T>&&>(self).unwrap_unchecked(unsafe);
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, meta::uncvref_t<T> const&>) &&
					VEG_CONCEPT(constructible<
											bool,
											meta::invoke_result_t<Fn, meta::uncvref_t<T> const&>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto filter,
			(fn, Fn&&)) &&

			noexcept(
					(VEG_CONCEPT(nothrow_invocable<Fn&&, meta::uncvref_t<T> const&>) &&
	         VEG_CONCEPT(nothrow_move_constructible<T>))) -> option<T> {
		auto&& self = static_cast<option<T>&&>(*this);
		if (self) {
			if (invoke(VEG_FWD(fn), self.as_cref().unwrap_unchecked(unsafe))) {
				return {some, VEG_FWD(self).as_ref().unwrap_unchecked(unsafe)};
			}
		}
		return none;
	}
};

template <typename T, bool = VEG_CONCEPT(copy_constructible<T>)>
struct option_copy_interface_base {

	VEG_CPP14(constexpr)
	auto clone() const& noexcept(VEG_CONCEPT(nothrow_copy_constructible<T>))
			-> option<T> {
		auto const& self = static_cast<option<T> const&>(*this);
		return self.as_ref().map(into_fn<T>{});
	}
};

template <typename T>
struct option_copy_interface_base<T, false> {};
template <typename T>
struct option_move_interface_base<T, false> {};

template <
		typename T,
		bool B1 = VEG_CONCEPT(copy_constructible<T>),
		bool B2 = VEG_CONCEPT(move_constructible<T>)>
struct option_copymove_interface_base : option_copy_interface_base<T, B1>,
																				option_move_interface_base<T, B2> {};

template <typename T>
struct option_copymove_interface_base<T, true, true>
		: option_copy_interface_base<T, true>, option_move_interface_base<T, true> {
	using option_copy_interface_base<T, true>::clone;
	using option_move_interface_base<T, true>::clone;
};

template <typename T, typename U, typename Fn>
HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto cmp(
		option<T> const& lhs, option<U> const& rhs, Fn fn) noexcept -> bool {
	if (lhs) {
		if (rhs) {
			return static_cast<bool>(
					fn(lhs.as_cref().unwrap_unchecked(unsafe),
			       rhs.as_cref().unwrap_unchecked(unsafe)));
		}
	}
	return (static_cast<bool>(lhs) == static_cast<bool>(rhs));
}

#define VEG_CMP(op, fn, ...)                                                   \
	VEG_TEMPLATE(                                                                \
			(typename T, typename U),                                                \
			requires(VEG_CONCEPT(__VA_ARGS__)),                                      \
			VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto             \
			operator op,                                                             \
			(lhs, option<T> const&),                                                 \
			(rhs, option<U> const&))                                                 \
	noexcept->bool {                                                             \
		return option_::cmp(lhs, rhs, fn);                                         \
	}                                                                            \
	static_assert(true, "")

VEG_CMP(==, cmp_equal, equality_comparable_with<T, U>);
VEG_CMP(<, cmp_less, partially_ordered_with<T, U>);
VEG_CMP(>, cmp_greater, partially_ordered_with<U, T>);
VEG_CMP(<=, cmp_less_equal, partially_ordered_with<U, T>);
VEG_CMP(>=, cmp_greater_equal, partially_ordered_with<T, U>);

#undef VEG_CMP

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(equality_comparable_with<T, U>)),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
		operator!=,
		(a, option<T> const&),
		(b, option<U> const&))
noexcept -> bool {
	return !option_::cmp(a, b, cmp_equal);
}

template <typename T>
struct option_flatten_base {};

template <typename T>
struct ret_none {
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto operator()() const noexcept
			-> option<T> {
		return none;
	}
};

template <typename T>
struct option_flatten_base<option<T>> {
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto flatten() && noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>)) -> option<T> {
		auto&& self = static_cast<option<option<T>>&&>(*this);
		return VEG_FWD(self).as_ref().map_or_else(
				into_fn<option<T>>{}, ret_none<T>{});
	}
};

template <typename T>
struct cmp_with_fn {
	meta::uncvref_t<T> const& rhs;
	template <typename U>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(U const& lhs) const noexcept
			-> bool {
		return cmp_equal(lhs, rhs);
	}
};

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD option
		: private internal::option_::option_move_ctor_base2<T>,
			public internal::option_::option_copymove_interface_base<T>,
			public internal::option_::option_flatten_base<T> {

	using internal::option_::option_move_ctor_base2<T>::option_move_ctor_base2;
	option() = default;
	VEG_DEFAULTS(option);

	VEG_CPP14(constexpr)
	option // NOLINT(hicpp-explicit-conversions)
			(none_t /*tag*/) noexcept
			: option{} {}

	VEG_CPP14(constexpr) auto operator=(none_t /*arg*/) noexcept -> option& {
		if (*this) {
			this->destroy();
		}
		return *this;
	}

	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(equality_comparable_with<T, U>)),
			VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto contains,
			(val, U const&))
	const noexcept->bool {
		return as_cref().map_or(internal::option_::cmp_with_fn<U>{val}, false);
	}

	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					(VEG_CONCEPT(invocable<Fn, Args...>) &&
	         VEG_SAME_AS(T, (meta::invoke_result_t<Fn, Args...>)))),
			VEG_CPP14(constexpr) void emplace_with,
			(fn, Fn),
			(... args, Args)) &

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>)) {
		if (*this) {
			*this = none;
		}
		this->_emplace(VEG_FWD(fn), VEG_FWD(args)...);
	}

	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args...>)),
			VEG_CPP14(constexpr) void emplace,
			(... args, Args)) &

			noexcept(VEG_CONCEPT(nothrow_constructible<T, Args...>)) {
		if (*this) {
			*this = none;
		}
		this->_arg_emplace(VEG_FWD(args)...);
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable<Fn, T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map,
			(fn, Fn&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn, T>))
					-> option<meta::invoke_result_t<Fn, T>> {

		if (*this) {
			return option<meta::invoke_result_t<Fn, T>>{
					some, inplace, VEG_FWD(fn), static_cast<option&&>(*this)._get()};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T>) &&
					meta::is_option<meta::invoke_result_t<Fn, T>>::value),
			VEG_NODISCARD VEG_CPP14(constexpr) auto and_then,
			(fn, Fn&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn, T>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return invoke(
					VEG_FWD(fn),
					static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename D),
			requires(
					(VEG_CONCEPT(invocable<Fn, T>) && //
	         VEG_CONCEPT(invocable<D>) &&     //
	         VEG_SAME_AS(
							 (meta::invoke_result_t<Fn, T>), (meta::invoke_result_t<D>)))),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or_else,
			(fn, Fn&&),
			(d, D&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return invoke(
					VEG_FWD(fn),
					static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
		}
		return invoke(VEG_FWD(d));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T>) &&
					VEG_CONCEPT(move_constructible<meta::invoke_result_t<Fn, T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or,
			(fn, Fn&&),
			(d, meta::invoke_result_t<Fn, T>)) &&

			noexcept(
					VEG_CONCEPT(
							nothrow_move_constructible<meta::invoke_result_t<Fn, T>>) &&
					VEG_CONCEPT(nothrow_invocable<Fn, T>))
					-> meta::invoke_result_t<Fn, T> {
		if (!*this) {
			return VEG_FWD(d);
		}
		return invoke(
				VEG_FWD(fn),
				static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires VEG_CONCEPT(invocable<Fn>) &&
					VEG_SAME_AS((meta::invoke_result_t<Fn>), veg::option<T>),
			VEG_NODISCARD VEG_CPP14(constexpr) auto or_else,
			(fn, Fn&&)) &&

			noexcept(
					(VEG_CONCEPT(nothrow_invocable<Fn>) &&
	         VEG_CONCEPT(nothrow_copy_constructible<T>))) -> option<T> {
		if (*this) {
			return {
					some, static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe)};
		}
		return invoke(VEG_FWD(fn));
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_cref() const noexcept
			-> option<meta::uncvref_t<T> const&> {
		if (*this) {
			return {some, this->_get()};
		}
		return none;
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit
	operator bool() const noexcept {
		return this->is_engaged();
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() & noexcept -> option<T&> {
		if (*this) {
			return {some, this->_get()};
		}
		return none;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() const& noexcept -> option<T const&> {
		if (*this) {
			return {some, this->_get()};
		}
		return none;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() && noexcept -> option<T&&> {
		if (*this) {
			return {some, static_cast<option&&>(*this)._get()};
		}
		return none;
	}

private:
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void unwrap_assert() const noexcept {
		bool holds_value = static_cast<bool>(*this);
		VEG_ASSERT(holds_value);
	}

	template <typename U, bool B>
	friend struct internal::option_::value_sentinel_impl;

	friend struct internal::option_::option_copy_interface_base<T>;
	friend struct internal::option_::option_move_interface_base<T>;
};
VEG_CPP17(

		template <typename T> option(some_t, T) -> option<T>;
		template <typename T> option(some_ref_t, T&&) -> option<T&&>;

)

template <typename T>
struct meta::value_sentinel_for<option<T>>
		: veg::internal::option_::value_sentinel_impl<T> {};

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_OPTION_HPP_8NVLXES2S */
