#define DOCTEST_CONFIG_IMPLEMENT

#include "doctest.h"
#include <cstdio>
#include <backward.hpp>

namespace veg {
int argc = 0;
char** argv = nullptr;
} // namespace veg

auto main(int argc, char** argv) -> int {
	veg::argc = argc;
	veg::argv = argv;

	char argname[] = "--veg-death-assertion-id";
	if (argc >= 3 && //
	    (std::strcmp(argv[argc - 2], argname) == 0)) {
		std::FILE* _{};
		_ = std::freopen("/dev/null", "w", stderr);
		(void)_;
		_ = std::freopen("/dev/null", "w", stdout);
		(void)_;
	} else {
		backward::SignalHandling const sh;
	}
	return doctest::Context(argc, argv).run();
}
