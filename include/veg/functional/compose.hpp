#include <veg/type_traits/invocable.hpp>
#include <veg/type_traits/constructible.hpp>
#include <veg/internal/prologue.hpp>

namespace veg {
inline namespace VEG_ABI {
namespace fn {
namespace internal {
namespace compose_ {
template <typename... Ts>
struct ComposeImpl;

template <>
struct ComposeImpl<> {
	template <typename T>
	static constexpr auto call(ComposeImpl<> /*self*/, T&& arg) noexcept -> T&& {
		return VEG_FWD(arg);
	}
};

template <typename First, typename... Rest>
struct ComposeImpl<First, Rest...> {
	First first;
	ComposeImpl<Rest...> rest;

	template <typename Self, typename... Args>
	using return_type_t =
			decltype((VEG_DECLVAL(Self &&).first)((ComposeImpl<Rest...>::call)(
					VEG_DECLVAL(Self &&).rest, VEG_DECLVAL(Args&&)...)));

	VEG_TEMPLATE(
			(typename Self, typename... Args),
			requires(
					VEG_CONCEPT(detected<
											ComposeImpl<Rest...>::template return_type_t,
											Args&&...>) &&
					VEG_CONCEPT(invocable<
											First,
											meta::detected_t<
													ComposeImpl<Rest...>::template return_type_t,
													Args&&...>>)),
			static constexpr auto call,
			(self, Self&&),
			(... args, Args&&))
	noexcept(noexcept((VEG_FWD(self).first)(
			(ComposeImpl<Rest...>::call)(VEG_FWD(self).rest, VEG_FWD(args)...))))

			-> meta::detected_t<return_type_t, Self, Args&&...> {
		return (VEG_FWD(self).first)(
				(ComposeImpl<Rest...>::call)(VEG_FWD(self).rest, VEG_FWD(args)...));
	}
};

} // namespace compose_
} // namespace internal

template <typename... Fns>
struct Compose {
	using Impl = internal::compose_::ComposeImpl<Fns...>;

	Impl impl;

	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(
					detected<Impl::template return_type_t, Impl const&, Args&&...>)),
			auto constexpr
			operator(),
			(... args, Args&&))
	const& noexcept(noexcept(Impl::call(impl, VEG_FWD(args)...)))
			->meta::detected_t<Impl::template return_type_t, Impl const&, Args&&...> {
		return Impl::call(impl, VEG_FWD(args)...);
	}

	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(
					detected<Impl::template return_type_t, Impl&&, Args&&...>)),
			auto constexpr
			operator(),
			(... args, Args&&))
	&&noexcept(noexcept(Impl::call(VEG_FWD(impl), VEG_FWD(args)...)))
				->meta::detected_t<Impl::template return_type_t, Impl&&, Args&&...> {
		return Impl::call(VEG_FWD(impl), VEG_FWD(args)...);
	}

	VEG_TEMPLATE(
			typename... Args,
			requires(VEG_CONCEPT(
					detected<Impl::template return_type_t, Impl&, Args&&...>)),
			auto constexpr
			operator(),
			(... args, Args&&))
	&noexcept(noexcept(Impl::call(impl, VEG_FWD(args)...)))
			 ->meta::detected_t<Impl::template return_type_t, Impl&, Args&&...> {
		return Impl::call(impl, VEG_FWD(args)...);
	}

	template <typename... Args>
	void operator()(Args&&...) const&& = delete;
};

namespace nb {
struct compose_fwd {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Fns>))),
			constexpr auto
			operator(),
			(... fns, Fns&&))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Fns>)))
			->Compose<Fns...> {
		return {VEG_FWD(fns)...};
	}
};
} // namespace nb
VEG_NIEBLOID(compose_fwd);
} // namespace fn
} // namespace VEG_ABI
} // namespace veg

#include <veg/internal/epilogue.hpp>
