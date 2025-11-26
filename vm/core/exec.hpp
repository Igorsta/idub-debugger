#pragma once

#include <cstddef>
#include "../config/config.hpp"
#include <unordered_map>
#include "../memory/memory.hpp"
#include "inline.h"

struct instrutction_t;
struct frame_t;
struct function_t;

using func_map = std::unordered_map<func_id_t, function_t>;

#define CONST_RAW_EXEC_ARGS                                                                        \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const func_map *avail_funcs

namespace _N_EXEC {
#define RAW_EXEC_ARGS                                                                              \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *mem_space,       \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] func_map *avail_funcs
#define FWD_RAW_ARGS instr, mem_space, frame, avail_funcs
using ret_t = std::ptrdiff_t;
using func_t = ret_t(RAW_EXEC_ARGS);

}; // namespace _N_EXEC

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


namespace _N_EXEC_UTILS {
INLINE static unit *last_on_stck(CONST_RAW_EXEC_ARGS) {
	CORE_ASSERT(frame->stack_start < frame->stack_head,
				"Trying to access part of the stack what belongs to other function");
	return (frame->stack_head - 1);
}

INLINE static unit &get_reg(unit arg0, CONST_RAW_EXEC_ARGS) {
	auto cur_func = frame->cur_func_id;
	auto &all_func = *avail_funcs;

	auto it = all_func.find(cur_func);
	CORE_ASSERT(it != all_func.end(),
				"Weird, currently executed function (id: {}) is not among the functions of thread",
				cur_func);

	auto &function = it->second;
	CORE_ASSERT(arg0 < function.regs.size(),
				"Trying to access registr out of bounds: function has {} registers and optcode "
				"asks for {}",
				function.regs.size(), arg0);

	return function.regs[arg0];
}
} // namespace _N_EXEC_UTILS