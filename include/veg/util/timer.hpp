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

struct raii_timer {
private:
	struct raw_parts {
		i64 begin;
		i64 end;
		fn::once_fn_view<fn::nothrow<void(i64)>> fn;
	} self;

public:
	explicit raii_timer(fn::once_fn_view<fn::nothrow<void(i64)>> fn) noexcept
			: self{monotonic_nanoseconds_since_epoch(), 0, VEG_FWD(fn)} {}

	~raii_timer() {
		i64 dt = monotonic_nanoseconds_since_epoch() - self.begin;
		VEG_FWD(self).fn(dt);
	}
	raii_timer(raii_timer const&) = delete;
	raii_timer(raii_timer&&) = delete;
	auto operator=(raii_timer const&) -> raii_timer& = delete;
	auto operator=(raii_timer&&) -> raii_timer& = delete;
};
} // namespace VEG_ABI
} // namespace time
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TIMER_HPP_2SVNQDV6S */
