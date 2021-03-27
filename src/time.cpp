#include "veg/util/timer.hpp"
#include <chrono>
#include <cinttypes>
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace time {
void log_elapsed_time(i64 duration, char const* msg, std::FILE* out) noexcept {
	i64 dt_unit[4] = {};

	for (i64& i : dt_unit) {
		i = duration % i64(1000);
		duration /= i64(1000);
	}

	std::fprintf(
			out,
			"done: %s: "
			"%3" PRId64 "s "
			"%3" PRId64 "ms "
			"%3" PRId64 "µs "
			"%3" PRId64 "ns\n",
			msg,
			dt_unit[3],
			dt_unit[2],
			dt_unit[1],
			dt_unit[0]);
}
auto monotonic_nanoseconds_since_epoch() noexcept -> i64 {
	return static_cast<i64>(
			std::chrono::duration_cast<std::chrono::nanoseconds>(
					std::chrono::steady_clock::now().time_since_epoch())
					.count());
}
} // namespace time
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
