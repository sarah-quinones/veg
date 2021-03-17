#ifndef __VEG_TIMER_HPP_2SVNQDV6S
#define __VEG_TIMER_HPP_2SVNQDV6S

#include "veg/internal/typedefs.hpp"
#include <cstdio>

namespace veg {

auto monotonic_nanoseconds_since_epoch() noexcept -> i64;

struct raii_timer {
private:
	struct layout {
		i64 begin;
		i64 end;
		std::FILE* file;
		char const* msg = "";
	} self;

public:
	explicit raii_timer(char const* msg = "", std::FILE* out = stdout) noexcept
			: self{monotonic_nanoseconds_since_epoch(), 0, out, msg} {}

	~raii_timer();
	raii_timer(raii_timer const&) = delete;
	raii_timer(raii_timer&&) = delete;
	auto operator=(raii_timer const&) -> raii_timer& = delete;
	auto operator=(raii_timer&&) -> raii_timer& = delete;
};
} // namespace veg

#endif /* end of include guard __VEG_TIMER_HPP_2SVNQDV6S */
