#ifndef VEG_CURRY_HPP_KV8QZWDES
#define VEG_CURRY_HPP_KV8QZWDES

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/internal/tuple_generic.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

namespace internal {
namespace _fn {

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn, Args..., StoredArgs...>>
VEG_INLINE static constexpr auto call_curried_once(
		Fn&& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>&& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Args..., StoredArgs...>)) -> Ret {
	return VEG_FWD(fn)(
			VEG_FWD(args)...,
			static_cast<StoredArgs&&>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf)...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn, StoredArgs..., Args...>>
VEG_INLINE static constexpr auto call_rcurried_once(
		Fn&& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>&& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn_once<Fn, Ret, StoredArgs..., Args...>)) -> Ret {
	return VEG_FWD(fn)(
			static_cast<StoredArgs&&>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf)...,
			VEG_FWD(args)...);
}

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn&, Args..., RefMut<StoredArgs>...>>
VEG_INLINE static constexpr auto call_curried_mut(
		Fn& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(
				nothrow_fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>)) -> Ret {
	return fn(
			VEG_FWD(args)...,
			RefMut<StoredArgs>{
					AsMut{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf,
			}...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn&, RefMut<StoredArgs>..., Args...>>
VEG_INLINE static constexpr auto call_rcurried_mut(
		Fn& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(
				nothrow_fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>)) -> Ret {
	return fn(
			RefMut<StoredArgs>{
					AsMut{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf,
			}...,
			VEG_FWD(args)...);
}

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret =
				meta::invoke_result_t<Fn const&, Args..., Ref<StoredArgs>...>>
VEG_INLINE static constexpr auto call_curried(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., Ref<StoredArgs>...>)) -> Ret {
	return fn(
			VEG_FWD(args)...,
			Ref<StoredArgs>{
					AsRef{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored).leaf,
			}...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret =
				meta::invoke_result_t<Fn const&, Ref<StoredArgs>..., Args...>>
VEG_INLINE static constexpr auto call_rcurried(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn<Fn, Ret, Ref<StoredArgs>..., Args...>)) -> Ret {
	return fn(
			Ref<StoredArgs>{
					AsRef{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored).leaf,
			}...,
			VEG_FWD(args)...);
}

} // namespace _fn
} // namespace internal

namespace fn {

template <typename Fn, typename... StoredArgs>
struct CurriedOnce {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn, Args..., StoredArgs...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, Args..., StoredArgs...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(
					nothrow_fn_once<Fn, Ret, Args..., StoredArgs...>)) -> Ret {
		return internal::_fn::call_curried_once(
				VEG_FWD(fn), VEG_FWD(stored_args), VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct RCurriedOnce {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn, StoredArgs..., Args...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, StoredArgs..., Args...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(
					nothrow_fn_once<Fn, Ret, StoredArgs..., Args...>)) -> Ret {
		return internal::_fn::call_rcurried_once(
				VEG_FWD(fn), VEG_FWD(stored_args), VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct CurriedMut {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn&, Args..., RefMut<StoredArgs>...>),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>))
			->Ret {
		return internal::_fn::call_curried_mut(fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct RCurriedMut {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn&, RefMut<StoredArgs>..., Args...>),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>))
			->Ret {
		return internal::_fn::call_rcurried_mut(fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct Curried {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn const&, Args..., Ref<StoredArgs>...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Args..., Ref<StoredArgs>...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., Ref<StoredArgs>...>))
			->Ret {
		return internal::_fn::call_curried(fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct RCurried {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn const&, Ref<StoredArgs>..., Args...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Ref<StoredArgs>..., Args...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, Ref<StoredArgs>..., Args...>))
			->Ret {
		return internal::_fn::call_rcurried(fn, stored_args, VEG_FWD(args)...);
	}
};

namespace nb {
struct curry_once {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->CurriedOnce<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct rcurry_once {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->RCurriedOnce<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};

struct curry_mut {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->CurriedMut<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct rcurry_mut {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->RCurriedMut<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};

struct curry {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->Curried<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct rcurry {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->RCurried<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
} // namespace nb
VEG_NIEBLOID(curry_once);
VEG_NIEBLOID(rcurry_once);
VEG_NIEBLOID(curry_mut);
VEG_NIEBLOID(rcurry_mut);
VEG_NIEBLOID(curry);
VEG_NIEBLOID(rcurry);
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CURRY_HPP_KV8QZWDES */
