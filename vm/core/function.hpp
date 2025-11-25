#pragma once

#include "exec.hpp"
#include <vector>

struct instrutction_t {
	constexpr static size_t args_per_instr = 2;
	_N_EXEC::func_t *action;
	unit arg[args_per_instr];

	instrutction_t(_N_EXEC::func_t *action) : action{action} {
		for (int i = 0; i < args_per_instr; i++) {
			arg[i] = 0;
		}
	}
};
using code_block = std::vector<instrutction_t>;

struct function_t {
	code_block body;
	uint64_t no_of_args;
	uint64_t res_size;
	mutable std::vector<unit> regs;
};
