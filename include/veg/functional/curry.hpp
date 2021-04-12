#ifndef VEG_CURRY_HPP_KV8QZWDES
#define VEG_CURRY_HPP_KV8QZWDES

#include <veg/type_traits/invocable.hpp>
#include <veg/type_traits/constructible.hpp>
#include <veg/internal/tuple_generic.hpp>
#include <veg/internal/prologue.hpp>

namespace veg {
inline namespace VEG_ABI {
namespace internal {
namespace curry_ {} // namespace curry_
} // namespace internal

namespace fn {
template <typename Fn, typename... StoredArgs>
struct Curried {
	Fn fn;
	veg::internal::SimpleTuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			typename... Args,
			requires(
					VEG_CONCEPT(move_constructible<Fn>) &&
					VEG_ALL_OF(VEG_CONCEPT(move_constructible<Args>))),
			constexpr auto push_fwd,
			(... args, Args&&))
	&&noexcept(
				VEG_CONCEPT(nothrow_move_constructible<Fn>) &&
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Args>)))
				->Curried<Fn, Args..., StoredArgs...> {
		return push_fwd_impl(VEG_FWD(args)..., VEG_FWD(stored_args));
	}

	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(invocable<Fn, Args&&..., StoredArgs&&...>)),
			constexpr auto
			operator(),
			(... args,
	     Args&&)) && noexcept(VEG_FWD(fn)(VEG_FWD(args)..., VEG_DECLVAL_NOEXCEPT(StoredArgs&&)...))
			-> meta::invoke_result_t<Fn, Args&&..., StoredArgs&&...> {
		return Curried::call(
				meta::make_index_sequence<sizeof...(StoredArgs)>{},
				meta::type_sequence<StoredArgs&&...>{},
				static_cast<Curried&&>(*this),
				VEG_FWD(args)...);
	}
	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(invocable<Fn, Args&&..., StoredArgs&...>)),
			constexpr auto
			operator(),
			(... args,
	     Args&&)) & noexcept(fn(VEG_FWD(args)..., VEG_DECLVAL_NOEXCEPT(StoredArgs&)...))
			-> meta::invoke_result_t<Fn&, Args&&..., StoredArgs&...> {
		return Curried::call(
				meta::make_index_sequence<sizeof...(StoredArgs)>{},
				meta::type_sequence<StoredArgs&...>{},
				*this,
				VEG_FWD(args)...);
	}
	VEG_TEMPLATE(
			typename... Args,
			requires(
					VEG_CONCEPT(invocable<Fn const&, Args&&..., StoredArgs const&...>)),
			constexpr auto
			operator(),
			(... args, Args&&))
	const& noexcept(fn(VEG_FWD(args)...,
	                   VEG_DECLVAL_NOEXCEPT(StoredArgs const&)...))
			->meta::invoke_result_t<Fn&, Args&&..., StoredArgs const&...> {
		return Curried::call(
				meta::make_index_sequence<sizeof...(StoredArgs)>{},
				meta::type_sequence<StoredArgs const&...>{},
				*this,
				VEG_FWD(args)...);
	}

private:
	template <
			bool NoExcept,
			typename Ret,
			typename Self,
			typename... Args,
			typename... Actual_Stored,
			usize... Is>
	static constexpr auto call(
			meta::index_sequence<Is...> /*iseq*/,
			meta::type_sequence<Actual_Stored...> /*tseq*/,
			Self&& self,
			Args&&... args) noexcept(NoExcept) -> Ret {
		return (VEG_FWD(self).fn)(
				VEG_FWD(args)...,
				static_cast<Actual_Stored&&>(
						internal::tup_::get_impl<Is>(self.stored_args))...);
	}

	template <typename... Args, usize... Is, typename... Ts>
	constexpr auto push_fwd_impl(
			Args&&... args,
			internal::tup_::IndexedTuple<meta::index_sequence<Is...>, Ts...>&&
					stored) //
			&& noexcept(
					VEG_CONCEPT(nothrow_move_constructible<Fn>) &&
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Args>)))
					-> Curried<Fn, Args..., StoredArgs...> {
		return {
				VEG_FWD(fn),
				{Cvt{},
		     VEG_FWD(args)...,
		     static_cast<Ts&&>(internal::tup_::get_impl<Is>(stored))...}};
	}
};
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include <veg/internal/epilogue.hpp>
#endif /* end of include guard VEG_CURRY_HPP_KV8QZWDES */
