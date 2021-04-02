#ifndef VEG_ASSERT_DEATH_HPP_BXMINZAES
#define VEG_ASSERT_DEATH_HPP_BXMINZAES

#include <veg/fn_view.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <vector>
#include <string>

#ifdef __unix

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>

#else
#error "not yet supported"
#endif

namespace veg {

extern int argc;
extern char** argv;

inline auto is_terminating_impl(fn::fn_once_view<void()> fn, char const* id)
		-> int {
	char argname[] = "--veg-death-assertion-id";

	if (argc >= 3 && //
	    (std::strcmp(argv[argc - 2], argname) == 0)) {
		if (std::strcmp(argv[argc - 1], id) != 0) {
			return 0; // unrelated child
		}
		VEG_FWD(fn)();
		std::exit(0);
	} else {
		auto uargc = usize(veg::argc);

		std::vector<char*> args;
		args.reserve(uargc + 3);

		std::string argvalue(id);

		for (usize i = 0; i < usize(uargc); ++i) {
			args.emplace_back(argv[i]);
		}
		args.emplace_back(&argname[0]);
		args.emplace_back(&argvalue[0]);
		args.emplace_back(nullptr);

		pid_t process_pid{};

		::posix_spawnp(
				&process_pid, argv[0], nullptr, nullptr, args.data(), environ);

		int stat{};
		pid_t res = ::waitpid(process_pid, &stat, 0);
		VEG_ASSERT(res == process_pid);
		if (stat == 0) {
			return 1;
		}
		return -1;
	}
}

inline auto is_terminating(fn::fn_once_view<void()> fn, char const* id)
		-> bool {
	return is_terminating_impl(VEG_FWD(fn), id) != 1;
}
inline auto is_non_terminating(fn::fn_once_view<void()> fn, char const* id)
		-> bool {
	return is_terminating_impl(VEG_FWD(fn), id) != -1;
}
} // namespace veg
#define CHECK_DEATH(...)                                                       \
	CHECK(::veg::is_terminating(                                                 \
			[&]() -> void __VA_ARGS__, __VEG_PP_STRINGIZE(__LINE__)))
#define CHECK_NO_DEATH(...)                                                    \
	CHECK(!::veg::is_terminating(                                                \
			[&]() -> void __VA_ARGS__, __VEG_PP_STRINGIZE(__LINE__)))

#endif /* end of include guard VEG_ASSERT_DEATH_HPP_BXMINZAES */
