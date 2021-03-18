#ifndef __VEG_TIMER_HPP_2SVNQDV6S
#define __VEG_TIMER_HPP_2SVNQDV6S

#include "veg/internal/typedefs.hpp"
#include "veg/fn_view.hpp"
#include <cstdio>

namespace veg {

auto monotonic_nanoseconds_since_epoch() noexcept -> i64;

namespace fn {
struct log_elapsed_time {
	explicit log_elapsed_time(
			char const* _msg = "", std::FILE* _out = stdout) noexcept
			: msg{_msg}, out{_out} {}

	char const* msg;
	std::FILE* out;

	void operator()(i64 duration) const noexcept {
		log_elapsed_time::impl(duration, msg, out);
	}

	static void impl(i64 duration, char const* msg, std::FILE* out) noexcept;
};
} // namespace fn

struct raii_timer {
private:
	struct raw_parts {
		i64 begin;
		i64 end;
		once_fn_view<void(i64) __VEG_CPP17(noexcept)> fn;
	} self;

public:
	explicit raii_timer(once_fn_view<void(i64)> fn) noexcept
			: self{monotonic_nanoseconds_since_epoch(), 0, VEG_FWD(fn)} {}

	~raii_timer() {
		i64 dt = monotonic_nanoseconds_since_epoch() - self.begin;
		VEG_FWD(self.fn)(dt);
	}
	raii_timer(raii_timer const&) = delete;
	raii_timer(raii_timer&&) = delete;
	auto operator=(raii_timer const&) -> raii_timer& = delete;
	auto operator=(raii_timer&&) -> raii_timer& = delete;
};
} // namespace veg

#endif /* end of include guard __VEG_TIMER_HPP_2SVNQDV6S */
