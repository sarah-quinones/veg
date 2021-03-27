#ifndef VEG_OPTION_HPP_8NVLXES2S
#define VEG_OPTION_HPP_8NVLXES2S

#include "veg/util/assert.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/internal/storage.hpp"
#include "veg/util/compare.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {

template <typename T>
struct option;

inline namespace tags {

struct none_t {
	friend constexpr auto operator==(none_t /*lhs*/, none_t /*rhs*/) noexcept
			-> bool {
		return true;
	}
	friend constexpr auto operator!=(none_t /*lhs*/, none_t /*rhs*/) noexcept
			-> bool {
		return false;
	}

private:
	constexpr none_t() = default;
	constexpr explicit none_t(none_t* /*unused*/) noexcept {}
	template <typename T>
	friend struct internal::meta_::static_const;
};
VEG_INLINE_VAR(none, none_t);

struct some_t {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(constructible<meta::uncvref_t<T>, T&&>)),
			VEG_CPP14(constexpr),
			auto
			operator(),
			(arg, T&&))
	const noexcept(VEG_CONCEPT(nothrow_constructible<meta::uncvref_t<T>, T&&>))
			->option<meta::uncvref_t<T>> {
		return {*this, VEG_FWD(arg)};
	}

private:
	constexpr some_t() = default;
	constexpr explicit some_t(some_t* /*unused*/) noexcept {}
	template <typename T>
	friend struct veg::internal::meta_::static_const;
};
VEG_INLINE_VAR(some, some_t);
} // namespace tags
namespace meta {

template <typename T>
struct is_mostly_trivial<option<T>> : is_mostly_trivial<T> {};
template <typename T>
struct is_trivially_relocatable<option<T>> : is_trivially_relocatable<T> {};

template <typename T>
struct is_option : false_type {};
template <typename T>
struct is_option<option<T>> : true_type {};

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
	friend struct veg::internal::meta_::static_const;
};
VEG_INLINE_VAR(some_ref, some_ref_t);

namespace internal {
namespace option_ {

struct hidden_tag {};

template <bool NoExcept, typename O>
VEG_CPP14(constexpr)
void copy_assign(O& lhs, O const& rhs) noexcept(NoExcept) {
	if (rhs.is_engaged()) {
		auto const& ref = rhs._get();
		if (lhs.is_engaged()) {
			lhs._get() = ref;
		} else {
			lhs._emplace(ref);
		}
	} else {
		if (lhs.is_engaged()) {
			lhs._destroy();
		}
	}
}

template <bool NoExcept, typename O>
VEG_CPP14(constexpr)
void move_assign(O& lhs, O&& rhs) noexcept(NoExcept) {
	if (rhs.is_engaged()) {
		if (lhs.is_engaged()) {
			lhs._get() = VEG_FWD(rhs)._get();
		} else {
			lhs._emplace(VEG_FWD(rhs)._get());
		}
	} else {
		if (lhs.is_engaged()) {
			lhs._destroy();
		}
	}
}

template <typename T>
using storage_for =
		meta::conditional_t<VEG_CONCEPT(reference<T>), storage_::storage<T>, T>;

enum triviality { mostly, dtor, no };

template <
		typename T,
		triviality = (VEG_CONCEPT(reference<T>) ||
                  VEG_CONCEPT(mostly_trivial<storage_for<T>>)) //
										 ? mostly
										 : VEG_CONCEPT(trivially_destructible<T>) ? dtor : no,
		bool = (meta::tombstone_traits<T>::spare_representations > 0)>
struct option_storage_base;

template <typename T>
struct disengage_fn {
	bool success;
	T& optional;
	VEG_CPP14(constexpr) void operator()() const noexcept {
		if (!success) {
			optional.disengage();
		}
	}
};

template <typename T>
struct option_storage_base<T, mostly, true> {
	using storage = storage_for<T>;
	using traits = meta::tombstone_traits<storage>;
#if __cplusplus >= 202002L
	storage some;
#else
	storage some = {};
#endif

	static constexpr i64 _spare_repr = traits::spare_representations - 1;
	static VEG_CPP14(constexpr) void _set_spare_repr(
			option<T>* p, i64 n) noexcept {
		static_assert(_spare_repr > 0, ".");
		traits::set_spare_representation(mem::addressof(p->some), n + 1);
	}
	static VEG_CPP14(constexpr) auto _index(option<T> const* p) noexcept -> i64 {
		static_assert(_spare_repr > 0, ".");
		return traits::index(mem::addressof(p->some)) - 1;
	}

	VEG_CPP14(constexpr) option_storage_base() noexcept { disengage(); }
	template <typename... Args>
	VEG_CPP14(constexpr)
	option_storage_base(hidden_tag /*unused*/, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>))
			: some(VEG_FWD(args)...) {}
	VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return traits::index(mem::addressof(some)) < 0;
	}
	VEG_CPP14(constexpr) void disengage() noexcept {
		traits::set_spare_representation(mem::addressof(some), 0);
	}
	template <typename... Args>
	VEG_CPP14(constexpr)
	void _emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {

		VEG_INTERNAL_ASSERT(!is_engaged());
		some = T(VEG_FWD(args)...);
	}
	VEG_CPP14(constexpr)
	void _destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		disengage();
	}
	VEG_CPP14(constexpr) auto _get() && noexcept -> T&& { return VEG_FWD(some); }
	VEG_CPP14(constexpr) auto _get() & noexcept -> T& { return some; }
	VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& { return some; }
};

template <typename T>
struct option_storage_tombstone {
	using traits = meta::tombstone_traits<T>;
	alignas(T) unsigned char buffer[sizeof(T)] = {};

	static constexpr i64 _spare_repr = traits::spare_representations - 1;
	static void _set_spare_repr(option<T>* p, i64 n) noexcept {
		static_assert(_spare_repr > 0, ".");
		traits::set_spare_representation(p->_ptr(), n + 1);
	}
	static auto _index(option<T> const* p) noexcept -> i64 {
		static_assert(_spare_repr > 0, ".");
		return traits::index(p->_ptr()) - 1;
	}

	option_storage_tombstone() noexcept { disengage(); }

	auto _ptr() const noexcept -> T* {
		using P = T*;
		return P(static_cast<unsigned char const*>(buffer));
	}

	template <typename... Args>
	option_storage_tombstone(hidden_tag /*unused*/, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {
		mem::construct_at(_ptr(), VEG_FWD(args)...);
	}

	VEG_NODISCARD auto is_engaged() const noexcept -> bool {
		return traits::index(_ptr()) < 0;
	}
	void disengage() noexcept { traits::set_spare_representation(_ptr(), 0); }
	template <typename... Args>
	void _emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {

		VEG_INTERNAL_ASSERT(!is_engaged());
		auto&& cleanup =
				make::defer(disengage_fn<option_storage_tombstone>{false, *this});
		mem::construct_at(_ptr(), VEG_FWD(args)...);
		cleanup.fn.success = true;
	}
	void _destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		mem::destroy_at(_ptr());
		disengage();
	}
	auto _get() && noexcept -> T&& { return static_cast<T&&>(*_ptr()); }
	auto _get() & noexcept -> T& { return *_ptr(); }
	auto _get() const& noexcept -> T const& { return *_ptr(); }
};

template <typename T>
struct option_storage_base<T, dtor, true> : option_storage_tombstone<T> {
	using option_storage_tombstone<T>::option_storage_tombstone;
};
template <typename T>
struct option_storage_base /* NOLINT */<T, no, true>
		: option_storage_tombstone<T> {
	using option_storage_tombstone<T>::option_storage_tombstone;
	~option_storage_base() noexcept {
		if (this->is_engaged()) {
			this->_destroy();
		}
	}
};

template <typename T>
struct option_storage_base<T, mostly, false> {
	unsigned char engaged = 0;
#if __cplusplus < 202002L
	T some{};
#else
	union {
		empty none = {};
		T some;
	};
#endif

	static constexpr i64 _spare_repr = i64{static_cast<unsigned char>(-3)};
	static VEG_CPP14(constexpr) void _set_spare_repr(
			option<T>* p, i64 n) noexcept {
		VEG_INTERNAL_ASSERT(n < +_spare_repr);
		p->engaged = static_cast<unsigned char>(2 + n);
	}
	static VEG_CPP14(constexpr) auto _index(option<T> const* p) noexcept -> i64 {
		return p->engaged - 2;
	}

	option_storage_base() noexcept = default;
	template <typename... Args>
	VEG_CPP14(constexpr)
	option_storage_base(hidden_tag /*unused*/, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>))
			: engaged(1), some(VEG_FWD(args)...) {}

	VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
		return engaged > 0;
	}
	template <typename... Args>
	VEG_CPP14(constexpr)
	void _emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {

		VEG_INTERNAL_ASSERT(!is_engaged());
		some = T(VEG_FWD(args)...);
		engaged = 1;
	}

	VEG_CPP14(constexpr)
	void _destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		engaged = 0;
	}
	VEG_CPP14(constexpr) auto _get() && noexcept -> T&& { return VEG_FWD(some); }
	VEG_CPP14(constexpr) auto _get() & noexcept -> T& { return some; }
	VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& { return some; }
};

template <typename T, typename Derived>
struct option_union_base {
	VEG_CPP20(constexpr) auto _self() const noexcept -> Derived& {
		return const_cast<Derived&>(static_cast<Derived const&>(*this));
	}

	static constexpr i64 _spare_repr = i64{static_cast<unsigned char>(-3)};
	static void _set_spare_repr(option<T>* p, i64 n) noexcept {
		VEG_INTERNAL_ASSERT(n < +_spare_repr);
		*reinterpret_cast<unsigned char*>(p) = static_cast<unsigned char>(2 + n);
	}
	static auto _index(option<T> const* p) noexcept -> i64 {
		return *reinterpret_cast<unsigned char const*>(p) - 2;
	}

	VEG_NODISCARD VEG_CPP20(constexpr) auto is_engaged() const noexcept -> bool {
		return _self().engaged > 0;
	}
	VEG_CPP20(constexpr) void disengage() noexcept {
		mem::construct_at(&_self().none);
	}
	template <typename... Args>
	VEG_CPP20(constexpr)
	void _emplace(Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>)) {

		VEG_INTERNAL_ASSERT(!is_engaged());
		auto&& cleanup = make::defer(disengage_fn<option_union_base>{false, *this});
		mem::destroy_at(&_self().none);
		mem::construct_at(mem::addressof(_self().some), VEG_FWD(args)...);
		_self().engaged = 1;
		cleanup.fn.success = true;
	}

	VEG_CPP20(constexpr)
	void _destroy() noexcept {
		VEG_INTERNAL_ASSERT(is_engaged());
		mem::destroy_at(mem::addressof(_self().some));
		_self().engaged = 0;
		disengage();
	}

	VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
		return VEG_FWD(_self().some);
	}
	VEG_CPP14(constexpr) auto _get() & noexcept -> T& { return _self().some; }
	VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
		return _self().some;
	}
};

template <typename T>
struct option_storage_base<T, dtor, false>
		: option_union_base<T, option_storage_base<T, dtor, false>> {
	unsigned char engaged = 0;
	union {
		empty none{};
		T some;
	};

	VEG_CPP14(constexpr) option_storage_base /* NOLINT */ () noexcept {};
	template <typename... Args>
	VEG_CPP20(constexpr)
	option_storage_base(hidden_tag /*unused*/, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>))
			: engaged(1), some(VEG_FWD(args)...) {}
};

template <typename T>
struct option_storage_base /* NOLINT */<T, no, false>
		: option_union_base<T, option_storage_base<T, no, false>> {
	unsigned char engaged = 0;
	union {
		empty none{};
		T some;
	};

	VEG_CPP14(constexpr) option_storage_base /* NOLINT */ () noexcept {};
	template <typename... Args>
	VEG_CPP20(constexpr)
	option_storage_base(hidden_tag /*unused*/, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, Args&&...>))
			: engaged(1), some(VEG_FWD(args)...) {}
	VEG_CPP20(constexpr) ~option_storage_base() noexcept {
		if (this->is_engaged()) {
			this->_destroy();
		}
	}
};

template <
		typename T,
		bool = VEG_CONCEPT(copy_constructible<T>) &&
					 !VEG_CONCEPT(trivially_copy_constructible<T>)>
struct option_copy_ctor;
template <
		typename T,
		bool = VEG_CONCEPT(move_constructible<T>) &&
					 !VEG_CONCEPT(trivially_move_constructible<T>)>
struct option_move_ctor;
template <
		typename T,
		bool = VEG_CONCEPT(copy_assignable<T>) &&
					 !VEG_CONCEPT(trivially_copy_assignable<T>)>
struct option_copy_assign;
template <
		typename T,
		bool = VEG_CONCEPT(move_assignable<T>) &&
					 !VEG_CONCEPT(trivially_move_assignable<T>)>
struct option_move_assign;

template <typename T>
struct option_copy_ctor<T, false> : option_storage_base<T> {
	using option_storage_base<T>::option_storage_base;
};
template <typename T>
struct option_move_ctor<T, false> : option_copy_ctor<T> {
	using option_copy_ctor<T>::option_copy_ctor;
};
template <typename T>
struct option_copy_assign<T, false> : option_move_ctor<T> {
	using option_move_ctor<T>::option_move_ctor;
};
template <typename T>
struct option_move_assign<T, false> : option_copy_assign<T> {
	using option_copy_assign<T>::option_copy_assign;
};

template <typename T>
struct option_copy_ctor<T, true> : option_storage_base<T> {
	using option_storage_base<T>::option_storage_base;
	~option_copy_ctor() = default;
	option_copy_ctor(option_copy_ctor&& rhs) = default;
	option_copy_ctor(option_copy_ctor const& rhs) noexcept(
			VEG_CONCEPT(nothrow_copy_constructible<T>))
			: option_storage_base<T>{} {
		if (rhs.is_engaged()) {
			auto const& ref = rhs._get();
			this->_emplace(ref);
		}
	}
	auto operator=(option_copy_ctor&& rhs) -> option_copy_ctor& = default;
	auto operator=(option_copy_ctor const& rhs) -> option_copy_ctor& = default;
};

template <typename T>
struct option_move_ctor<T, true> : option_copy_ctor<T> {
	using option_copy_ctor<T>::option_copy_ctor;
	~option_move_ctor() = default;
	option_move_ctor(option_move_ctor&& rhs) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>))
			: option_copy_ctor<T>{} {
		if (rhs.is_engaged()) {
			this->_emplace(VEG_FWD(rhs)._get());
		}
	}
	option_move_ctor(option_move_ctor const& rhs) = default;
	auto operator=(option_move_ctor&& rhs) -> option_move_ctor& = default;
	auto operator=(option_move_ctor const& rhs) -> option_move_ctor& = default;
};
template <typename T>
struct option_copy_assign<T, true> : option_move_ctor<T> {
	using option_move_ctor<T>::option_move_ctor;
	~option_copy_assign() = default;
	option_copy_assign(option_copy_assign&& rhs) = default;
	option_copy_assign(option_copy_assign const& rhs) = default;
	auto operator=(option_copy_assign&& rhs) -> option_copy_assign& = default;
	auto operator= /* NOLINT */(option_copy_assign const& rhs) noexcept(
			VEG_CONCEPT(nothrow_copy_constructible<T>) &&
			VEG_CONCEPT(nothrow_copy_assignable<T>)) -> option_copy_assign& {
		option_::copy_assign<noexcept(*this = rhs)>(*this, rhs);
		return *this;
	}
};
template <typename T>
struct option_move_assign<T, true> : option_copy_assign<T> {
	using option_copy_assign<T>::option_copy_assign;
	~option_move_assign() = default;
	option_move_assign(option_move_assign&& rhs) = default;
	option_move_assign(option_move_assign const& rhs) = default;
	auto operator=(option_move_assign&& rhs) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>) &&
			VEG_CONCEPT(nothrow_move_assignable<T>)) -> option_move_assign& {
		option_::move_assign<noexcept(*this = VEG_FWD(rhs))>(*this, VEG_FWD(rhs));
		return *this;
	}
	auto operator=(option_move_assign const& rhs)
			-> option_move_assign& = default;
};

template <typename T>
using choose_base = meta::conditional_t<
		VEG_CONCEPT(reference<T>),
		option_storage_base<T>,
		option_move_assign<T>>;

template <typename T, bool = VEG_CONCEPT(move_constructible<T>)>
struct option_move_ctor_base2 : choose_base<T> {

	option_move_ctor_base2() = default;
	explicit VEG_CPP14(constexpr) option_move_ctor_base2(T value) noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>))
			: choose_base<T>(hidden_tag{}, VEG_FWD(value)) {}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr)
			option_move_ctor_base2(some_t /*tag*/, T value) noexcept(
					VEG_CONCEPT(nothrow_move_constructible<T>))
			: choose_base<T>(hidden_tag{}, VEG_FWD(value)) {}

	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args...>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
			option_move_ctor_base2,
			(/*tag*/, some_t),
			(/*tag*/, inplace_t),
			(... args, Args&&))
	noexcept(VEG_CONCEPT(nothrow_constructible<T, Args...>))
			: choose_base<T>(internal::option_::hidden_tag{}, VEG_FWD(args)...) {}
};

template <typename T>
struct option_move_ctor_base2<T, false> : option_move_assign<T> {
	option_move_ctor_base2() = default;
	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args...>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
			option_move_ctor_base2,
			(/*tag*/, some_t),
			(/*tag*/, inplace_t),
			(... args, Args&&))
	noexcept(VEG_CONCEPT(nothrow_constructible<T, Args...>))
			: option_move_assign<T>(
						internal::option_::hidden_tag{}, VEG_FWD(args)...) {}
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
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto filter,
			(fn, Fn&&)) &&

			noexcept(
					(VEG_CONCEPT(nothrow_invocable<Fn&&, meta::uncvref_t<T> const&>) &&
	         VEG_CONCEPT(nothrow_move_constructible<T>))) -> option<T> {
		auto&& self = static_cast<option<T>&&>(*this);
		if (self) {
			if (VEG_FWD(fn)(self.as_cref().unwrap_unchecked(unsafe))) {
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

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
		auto
		operator==,
		(lhs, option<T> const&),
		(rhs, option<U> const&))
noexcept -> bool {
	return option_::cmp(lhs, rhs, veg::cmp::equal);
}

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
		auto
		operator!=,
		(a, option<T> const&),
		(b, option<U> const&))
noexcept -> bool {
	return !(a == b);
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

template <typename U>
struct cmp_with_fn {
	U const& rhs;
	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T const& lhs) const noexcept
			-> bool {
		return veg::cmp::equal(lhs, rhs);
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
	VEG_DEFAULT_CTOR_ASSIGN(option);

	VEG_CPP14(constexpr)
	option // NOLINT(hicpp-explicit-conversions)
			(none_t /*tag*/) noexcept
			: option{} {}

	VEG_CPP14(constexpr) auto operator=(none_t /*arg*/) noexcept -> option& {
		if (*this) {
			this->_destroy();
		}
		return *this;
	}

	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
			VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr),
			auto contains,
			(val, U const&))
	const noexcept->bool {
		return as_ref().map_or(internal::option_::cmp_with_fn<U>{val}, false);
	}

	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args...>)),
			VEG_CPP14(constexpr),
			void emplace,
			(... args, Args)) &

			noexcept(VEG_CONCEPT(nothrow_constructible<T, Args...>)) {
		if (*this) {
			*this = none;
		}
		this->_emplace(VEG_FWD(args)...);
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable<Fn, T&&>)),
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto map,
			(fn, Fn&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn, T&&>))
					-> option<meta::invoke_result_t<Fn, T>> {

		if (*this) {
			return option<meta::invoke_result_t<Fn, T>>{
					some,
					inplace,
					make::from_callable(
							VEG_FWD(fn), static_cast<option&&>(*this)._get())};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T&&>) &&
					meta::is_option<meta::invoke_result_t<Fn, T>>::value),
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto and_then,
			(fn, Fn&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn, T&&>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return VEG_FWD(fn)(
					static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename D),
			requires(
					(VEG_CONCEPT(invocable<Fn, T&&>) && //
	         VEG_CONCEPT(invocable<D>) &&       //
	         VEG_CONCEPT(
							 same<meta::invoke_result_t<Fn, T>, meta::invoke_result_t<D>>))),
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto map_or_else,
			(fn, Fn&&),
			(d, D&&)) &&

			noexcept(VEG_CONCEPT(nothrow_invocable<Fn>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return VEG_FWD(fn)(
					static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
		}
		return VEG_FWD(d)();
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T&&>) &&
					VEG_CONCEPT(move_constructible<meta::invoke_result_t<Fn, T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto map_or,
			(fn, Fn&&),
			(d, meta::invoke_result_t<Fn, T>)) &&

			noexcept(
					VEG_CONCEPT(
							nothrow_move_constructible<meta::invoke_result_t<Fn, T>>) &&
					VEG_CONCEPT(nothrow_invocable<Fn, T&&>))
					-> meta::invoke_result_t<Fn, T> {
		if (!*this) {
			return VEG_FWD(d);
		}
		return VEG_FWD(fn)(
				static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn>) &&
					VEG_CONCEPT(same<veg::option<T>, meta::invoke_result_t<Fn>>)),
			VEG_NODISCARD VEG_CPP14(constexpr),
			auto or_else,
			(fn, Fn&&)) &&

			noexcept(
					(VEG_CONCEPT(nothrow_invocable<Fn>) &&
	         VEG_CONCEPT(nothrow_copy_constructible<T>))) -> option<T> {
		if (*this) {
			return {
					some, static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe)};
		}
		return VEG_FWD(fn)();
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_cref() const noexcept
			-> option<meta::uncvref_t<T> const&> {
		if (*this) {
			return {some, this->_get()};
		}
		return {};
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) explicit
	operator bool() const noexcept {
		return this->is_engaged();
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() & noexcept -> option<T&> {
		if (this->is_engaged()) {
			return {some, this->_get()};
		}
		return {};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() const& noexcept -> option<T const&> {
		if (this->is_engaged()) {
			return {some, this->_get()};
		}
		return {};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) auto as_ref() && noexcept -> option<T&&> {
		if (this->is_engaged()) {
			return {some, static_cast<option&&>(*this)._get()};
		}
		return {};
	}

private:
	HEDLEY_ALWAYS_INLINE
	VEG_CPP14(constexpr) void unwrap_assert() const noexcept {
		bool holds_value = static_cast<bool>(*this);
		VEG_ASSERT(holds_value);
	}

	friend struct meta::tombstone_traits<option<T>>;
	friend struct internal::option_::option_storage_base<T>;
	friend struct internal::option_::option_storage_tombstone<T>;

	friend struct internal::option_::option_copy_interface_base<T>;
	friend struct internal::option_::option_move_interface_base<T>;
};
VEG_CPP17(

		template <typename T> option(some_t, T) -> option<T>;
		template <typename T> option(some_ref_t, T&&) -> option<T&&>;

)

template <typename T>
struct meta::tombstone_traits<option<T>> {
	static constexpr i64 spare_representations = option<T>::_spare_repr;

	HEDLEY_ALWAYS_INLINE
	static VEG_CPP14(constexpr) void set_spare_representation(
			option<T>* p, i64 n) noexcept {
		option<T>::_set_spare_repr(p, n);
	}
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) auto index(
			option<T> const* p) noexcept -> i64 {
		return option<T>::_index(p);
	}
};

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_OPTION_HPP_8NVLXES2S */
