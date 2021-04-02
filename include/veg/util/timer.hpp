#ifndef VEG_TIMER_HPP_2SVNQDV6S
#define VEG_TIMER_HPP_2SVNQDV6S

#include "veg/internal/typedefs.hpp"
#include "veg/fn_view.hpp"
#include <cstdio>
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace time {
auto monotonic_nanoseconds_since_epoch() noexcept -> i64;
void log_elapsed_time(i64 duration, char const* msg, std::FILE* out) noexcept;
} // namespace time
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace internal {
namespace time_ {
template <typename Fn>
struct raii_timer_wrapper {
	struct raw_parts {
		i64 begin;
		Fn fn;
	} self;

	raii_timer_wrapper(Fn fn) noexcept
			: self{abi::time::monotonic_nanoseconds_since_epoch(), VEG_FWD(fn)} {}

	auto operator()() noexcept(noexcept(VEG_FWD(self).fn)) {
		VEG_FWD(self.fn)
		(i64(abi::time::monotonic_nanoseconds_since_epoch() - self.begin));
	}
};
} // namespace time_
} // namespace internal

namespace time {
inline namespace VEG_ABI {
namespace nb {
struct monotonic_nanoseconds_since_epoch {
	auto operator()() const noexcept -> i64 {
		return abi::time::monotonic_nanoseconds_since_epoch();
	}
};
} // namespace nb
VEG_NIEBLOID(monotonic_nanoseconds_since_epoch);

struct log_elapsed_time {
	explicit log_elapsed_time(
			char const* _msg = "", std::FILE* _out = stdout) noexcept
			: msg{_msg}, out{_out} {}

	char const* msg;
	std::FILE* out;

	void operator()(i64 duration) const noexcept {
		abi::time::log_elapsed_time(duration, msg, out);
	}
};

template <typename Fn>
struct raii_timer : defer<internal::time_::raii_timer_wrapper<Fn>> {
	VEG_CHECK_CONCEPT(invocable<Fn, i64>);
	using defer<internal::time_::raii_timer_wrapper<Fn>>::defer;
	using defer<internal::time_::raii_timer_wrapper<Fn>>::fn;
};

namespace make {
namespace nb {
struct raii_timer {
	VEG_TEMPLATE(
			typename Fn,
			requires(
					VEG_CONCEPT(invocable<Fn, i64>) &&
					VEG_CONCEPT(move_constructible<Fn>)),
			auto
			operator(),
			(fn, Fn&&))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<Fn>))
			->time::raii_timer<Fn> {
		return {VEG_FWD(fn)};
	}
};
} // namespace nb
VEG_NIEBLOID(raii_timer);
} // namespace make
} // namespace VEG_ABI
} // namespace time
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TIMER_HPP_2SVNQDV6S */
