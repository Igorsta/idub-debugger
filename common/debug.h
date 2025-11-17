#pragma once
#include <print>
#include <cassert>

#define CORE_ASSERT(cond, ...)                                                                     \
	if (!(cond)) {                                                                                 \
		std::println(__VA_ARGS__);                                                                 \
		assert(false);                                                                             \
	}
