#ifndef VEG_ASSERT_DEATH_HPP_BXMINZAES
#define VEG_ASSERT_DEATH_HPP_BXMINZAES

#include "veg/fn_view.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>

#ifdef __unix

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#else
#error "not yet supported"
#endif

namespace veg {

inline auto is_terminating(fn::once_fn_view<void()> fn) -> bool {

#ifdef __unix

	pid_t pid = ::fork();
	switch (pid) {
	case -1: {
		std::terminate();
		break;
	}

	case 0: {
		{
			auto* _ = std::freopen("/dev/null", "w", stderr);
			(void)_;
		}
		{
			auto* _ = std::freopen("/dev/null", "w", stdout);
			(void)_;
		}
		std::set_terminate([] { std::abort(); });
		VEG_FWD(fn)();
		std::exit(0);
		break;
	}

	default:
		int stat{};
		pid_t res = ::waitpid(pid, &stat, 0);
		VEG_ASSERT(res == pid);
		return stat != 0;
	}

#else

	return true; // why not haha

#endif
}
} // namespace veg
#define CHECK_DEATH(...) CHECK(::veg::is_terminating([&]() -> void __VA_ARGS__))
#define CHECK_NO_DEATH(...)                                                    \
	CHECK(!::veg::is_terminating([&]() -> void __VA_ARGS__))

#endif /* end of include guard VEG_ASSERT_DEATH_HPP_BXMINZAES */
