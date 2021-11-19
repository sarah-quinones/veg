#include <memory>
#include <cassert>

#define FWD(x) static_cast<decltype(x)&&>(x)

template <typename Sig>
struct FnVTableCall;

template <typename R, typename... Args>
struct FnVTableCall<R(Args...)> {
	auto (*call)(void* data, Args...) -> R;

	template <typename F>
	static constexpr auto make_value() noexcept -> FnVTableCall {
		return {+[](void* data, Args... args) -> R {
			return (*static_cast<F*>(data))(FWD(args)...);
		}};
	};
};

template <typename... Sigs>
struct FnVTable : FnVTableCall<Sigs>... {
	void (*deleter)(void* data) noexcept;

	template <typename F>
	static constexpr auto make_value() noexcept -> FnVTable {
		return {
				FnVTableCall<Sigs>::template make_value<F>()...,
				+[](void* data) noexcept -> void { delete static_cast<F*>(data); },
		};
	}
};

template <typename D, typename Sig>
struct FnCrtp;
template <typename D, typename R, typename... Args>
struct FnCrtp<D, R(Args...)> {
	auto operator()(Args... args) -> R {
		auto& self = static_cast<D&>(*this);
		return static_cast<FnVTableCall<R(Args...)> const*>(self.raw.vtable)
		    ->call(self.raw.data, FWD(args)...);
	}
};

inline constexpr struct AsRef {
} as_ref;

template <typename F, typename... Sigs>
struct FnVTableObject {
	static constexpr auto value = FnVTable<Sigs...>::template make_value<F>();
};

template <typename... Sigs>
struct Fn : FnCrtp<Fn<Sigs...>, Sigs>... {
	using FnCrtp<Fn<Sigs...>, Sigs>::operator()...;

	struct Raw {
		FnVTable<Sigs...> const* vtable;
		void* data;
	} raw = {};

	Fn() = default;
	Fn(Fn&& rhs) noexcept : raw{rhs.raw} { rhs.raw = {}; }

	template <typename F>
	Fn(AsRef tag, F f)
			: raw{
						std::addressof(FnVTableObject<F, Sigs...>::value),
						new auto {FWD(f)},
				} {}

	~Fn() {
		if (raw.data != nullptr) {
			raw.vtable->deleter(raw.data);
		}
	}
};

template <typename... F>
struct overloaded : F... {
	using F::operator()...;
};
template <typename... F>
overloaded(F...) -> overloaded<F...>;

auto main() -> int {
	auto f = overloaded{
			[](int i) { return 2 * i; },
			[](float i) { return 4 * i; },
	};

	Fn<int(int), float(float)> a{as_ref, f};
	assert(a(3) == 6);
	assert(a(4.0F) == 16.0F);
	assert(false);
}
