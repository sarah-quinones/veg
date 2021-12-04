#ifndef VEG_OPTION_HPP_8NVLXES2S
#define VEG_OPTION_HPP_8NVLXES2S

#include "veg/util/assert.hpp"
#include "veg/util/unreachable.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/util/compare.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/uwunion.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

namespace internal {
namespace _option {
namespace adl {
struct AdlBase {};
} // namespace adl
} // namespace _option
} // namespace internal

template <typename T>
struct Option;

namespace option {
namespace nb {
struct some {
	template <typename T>
	VEG_NODISCARD VEG_INLINE constexpr auto operator()(T arg) const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> Option<T> {
		return {inplace[some{}], internal::MoveFn<T>{VEG_FWD(arg)}};
	}
};
} // namespace nb
VEG_NIEBLOID(some);
} // namespace option

inline namespace tags {
using Some = veg::option::nb::some;
using veg::option::some;

struct None : internal::_option::adl::AdlBase {};
VEG_INLINE_VAR(none, None);
} // namespace tags

namespace meta {
template <typename T>
struct is_option : false_type {};
template <typename T>
struct is_option<Option<T>> : true_type {};

template <typename T>
struct option_type : type_identity<internal::meta_::none> {};
template <typename T>
struct option_type<Option<T>> : type_identity<T> {};

template <typename T>
using option_type_t = typename option_type<T>::type;
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(typename T, option, meta::is_option<T>::value);
} // namespace concepts

namespace internal {
namespace _option {
template <typename To>
struct into_fn {
	template <typename T>
	VEG_INLINE constexpr auto operator()(T&& ref) const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, T&&>)) -> To {
		return VEG_FWD(ref);
	}
};

template <typename T>
struct ret_none {
	VEG_INLINE VEG_CPP14(constexpr) auto operator()() const VEG_NOEXCEPT -> T {
		return none;
	}
};

} // namespace _option
} // namespace internal

template <typename T>
struct VEG_NODISCARD Option
		: private internal::_uwunion::UwunionImpl<internal::Empty, T>,
			private internal::_option::adl::AdlBase {
private:
	using Base = internal::_uwunion::UwunionImpl<internal::Empty, T>;

public:
	constexpr Option() VEG_NOEXCEPT
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{0}>{},
						internal::_uwunion::IdxMoveFn<internal::Empty>{internal::Empty{}},
						usize{0},
				} {}
	VEG_EXPLICIT_COPY(Option);

	constexpr Option // NOLINT(hicpp-explicit-conversions)
			(None /*tag*/) VEG_NOEXCEPT : Option{} {}

	VEG_INLINE constexpr Option(Some /*tag*/, T value)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{1}>{},
						internal::_uwunion::IdxMoveFn<T>{VEG_FWD(value)},
						usize{1},
				} {}

	VEG_TEMPLATE(
			(typename _, typename Fn),
			requires(VEG_CONCEPT(same<_, Some>) && VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_INLINE constexpr Option,
			(/*tag*/, InPlace<_>),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, T>))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{1}>{},
						internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)},
						usize{1},
				} {}

	VEG_CPP14(constexpr)
	void reset() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		if (is_some()) {
			this->template _emplace<usize{0}>(
					internal::_uwunion::IdxMoveFn<internal::Empty>{internal::Empty{}});
		}
	}

	VEG_CPP14(constexpr)
	auto operator=(None /*arg*/)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) -> Option& {
		reset();
		return *this;
	}

	VEG_CONSTRAINED_MEMBER_FN_NO_PARAM(
			requires(VEG_CONCEPT(option<T>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto flatten,
			T,
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>))) {
		return static_cast<Option<T>&&>(*this).map_or_else(
				internal::_option::into_fn<T>{}, internal::_option::ret_none<T>{});
	}

private:
	VEG_CPP14(constexpr) auto _get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(Base::template get_ref<1>());
	}

public:
	VEG_NODISCARD VEG_CPP14(constexpr) auto take()
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> Option<T> {
		if (is_some()) {
			Option<T> val{
					inplace[some],
					internal::MoveFn<T>{static_cast<T&&>(this->_get())},
			};
			reset();
			return val;
		}
		return none;
	}

	VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap_unchecked(Unsafe /*tag*/) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		meta::unreachable_if(is_none());
		return static_cast<T&&>(this->_get());
	}

	VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap() &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		VEG_INTERNAL_ASSERT_PRECONDITION(is_some());
		return static_cast<T&&>(this->_get());
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(fn_once<Fn, bool, Ref<T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto filter,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_fn_once<Fn, bool, Ref<T>>) &&
	         VEG_CONCEPT(nothrow_movable<T>))) -> Option<T> {
		if (is_some() && VEG_FWD(fn)(ref(this->_get()))) {
			return {
					inplace[some],
					internal::MoveFn<T>{static_cast<T&&>(this->_get())},
			};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename U = T),
			requires(VEG_CONCEPT(eq<T, U>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto contains,
			(val, Ref<U>))
	const VEG_NOEXCEPT->bool {
		if (is_none()) {
			return false;
		}
		return cmp::eq(ref(_get()), val);
	}

	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_CPP14(constexpr) auto emplace,
			(fn, Fn)) &
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_destructible<T>) &&
					VEG_CONCEPT(nothrow_fn_once<Fn, T>)) -> T& {
		this->template _emplace<usize{1}>(
				internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)});
		return this->_get();
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>)) -> Option<Ret> {
		if (is_some()) {
			return Option<Ret>{
					inplace[some],
					internal::WithArg<Fn&&, T&&>{
							VEG_FWD(fn), static_cast<T&&>(this->_get())},
			};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(
					VEG_CONCEPT(fn_once<Fn, Ret, T>) && //
					VEG_CONCEPT(option<Ret>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto and_then,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>)) -> Ret {
		if (is_some()) {
			return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename D, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(
					VEG_CONCEPT(fn_once<Fn, Ret, T>) && //
					VEG_CONCEPT(fn_once<D, Ret>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or_else,
			(fn, Fn),
			(d, D)) &&

			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>) &&
					VEG_CONCEPT(nothrow_fn_once<D, Ret>)) -> Ret {
		if (is_some()) {
			return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
		}
		return VEG_FWD(d)();
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or,
			(fn, Fn),
			(d, Ret)) &&

			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>) &&
					VEG_CONCEPT(nothrow_movable<Ret>)) -> Ret {
		if (is_none()) {
			return VEG_FWD(d);
		}
		return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(fn_once<Fn, Option<T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto or_else,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_fn_once<Fn, Option<T>>) &&
	         VEG_CONCEPT(nothrow_movable<T>))) -> Option<T> {
		if (is_some()) {
			return {
					inplace[some],
					internal::MoveFn<T>{static_cast<T&&>(this->_get())},
			};
		}
		return VEG_FWD(fn)();
	}

	VEG_NODISCARD VEG_INLINE constexpr auto is_some() const VEG_NOEXCEPT -> bool {
		return usize(Base::index()) != 0;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto is_none() const VEG_NOEXCEPT -> bool {
		return !is_some();
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_cref() const
			VEG_NOEXCEPT -> Option<meta::uncvref_t<T> const&> {
		if (is_some()) {
			return {some, this->_get()};
		}
		return {};
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_mut() VEG_NOEXCEPT
			-> Option<RefMut<T>> {
		if (is_some()) {
			return {some, veg::mut(this->_get())};
		}
		return {};
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Option<Ref<T>> {
		if (is_some()) {
			return {some, ref(this->_get())};
		}
		return {};
	}
};
namespace internal {
namespace _option {
namespace adl {
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
		operator==,
		(lhs, Option<T> const&),
		(rhs, Option<U> const&))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	if (lhs.is_some() && rhs.is_some()) {
		return cmp::eq(
				lhs.as_ref().unwrap_unchecked(unsafe),
				rhs.as_ref().unwrap_unchecked(unsafe));
	}
	return (lhs.is_some() == rhs.is_some());
}
template <typename T>
VEG_NODISCARD VEG_INLINE constexpr auto
operator==(Option<T> const& lhs, None /*rhs*/) VEG_NOEXCEPT -> bool {
	return lhs.is_none();
}
template <typename U>
VEG_NODISCARD VEG_INLINE constexpr auto
operator==(None /*lhs*/, Option<U> const& rhs) VEG_NOEXCEPT -> bool {
	return rhs.is_none();
}
constexpr auto operator==(None /*lhs*/, None /*rhs*/) VEG_NOEXCEPT -> bool {
	return true;
}
} // namespace adl
} // namespace _option
} // namespace internal

namespace internal {
namespace _option {
struct DbgOptionBase {
	template <typename T>
	static void to_string(fmt::BufferMut out, Ref<Option<T>> opt) {
		if (opt.get().is_some()) {
			out.append_literal("some(");
			fmt::Debug<T>::to_string(
					VEG_FWD(out), opt.get().as_ref().unwrap_unchecked(unsafe));
			out.append_literal(")");
		} else {
			out.append_literal("none");
		}
	}
};
struct OrdOptionBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Option<T>>),
			(rhs, Ref<Option<U>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return (lhs.get().is_some()) //
		           ? rhs.get().is_some()
		                 ? static_cast<cmp::Ordering>(cmp::Ord<T, U>::cmp(
													 lhs.get().as_ref().unwrap_unchecked(unsafe),
													 rhs.get().as_ref().unwrap_unchecked(unsafe)))
		                 : cmp::Ordering::greater
		           // lhs is none
		           : rhs.get().is_none() ? cmp::Ordering::equal
		                                 : cmp::Ordering::less;
	}
};
struct OrdOptionBaseLhsNone {
	template <typename U>
	VEG_NODISCARD VEG_INLINE static constexpr auto
	cmp(Ref<None> /*lhs*/, Ref<Option<U>> rhs) VEG_NOEXCEPT -> cmp::Ordering {
		return rhs.get().is_none() ? cmp::Ordering::equal : cmp::Ordering::less;
	}
};
struct OrdOptionBaseRhsNone {
	template <typename T>
	VEG_NODISCARD VEG_INLINE static constexpr auto
	cmp(Ref<Option<T>> lhs, Ref<None> /*rhs*/) VEG_NOEXCEPT -> cmp::Ordering {
		return lhs.get().is_none() ? cmp::Ordering::equal : cmp::Ordering::greater;
	}
};
} // namespace _option
} // namespace internal

template <typename T>
struct fmt::Debug<Option<T>> : internal::_option::DbgOptionBase {};
template <>
struct fmt::Debug<None> {
	static void to_string(fmt::BufferMut out, Ref<None> /*unused*/) {
		out.append_literal("none");
	}
};
template <typename T, typename U>
struct cmp::Ord<Option<T>, Option<U>> : internal::_option::OrdOptionBase {};
template <typename U>
struct cmp::Ord<None, Option<U>> : internal::_option::OrdOptionBaseLhsNone {};
template <typename T>
struct cmp::Ord<Option<T>, None> : internal::_option::OrdOptionBaseRhsNone {};
template <>
struct cmp::Ord<None, None> : internal::_option::OrdOptionBaseRhsNone {
	VEG_NODISCARD VEG_INLINE static constexpr auto
	cmp(Ref<None> /*unused*/, Ref<None> /*unused*/) VEG_NOEXCEPT
			-> cmp::Ordering {
		return cmp::Ordering::equal;
	}
};

template <typename T>
struct cpo::is_trivially_constructible<Option<T>>
		: cpo::is_trivially_constructible<T> {};
template <typename T>
struct cpo::is_trivially_relocatable<Option<T>>
		: cpo::is_trivially_relocatable<T> {};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_OPTION_HPP_8NVLXES2S */
