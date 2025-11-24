#pragma once
#include "exec.hpp"
#include "args.hpp"
#include <regex>

struct thread_builder_t;

#define _N_PARSE	   parse
#define _N_PARSE_UTILS _N_PARSE::utils

namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using func_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE


#define REQUIRE_PARSE(macro)                                                                       \
	REQUIRE_ARGS(macro)                                                                            \
	namespace _N_PARSE {                                                                           \
	func_t macro;                                                                                  \
	}
