#pragma once

#include <cstddef>
#include "config.hpp"
#include <unordered_map>
#include "memory.hpp"

struct instrutction_t;
struct frame_t;
struct function_t;

using func_map = std::unordered_map<func_id_t, function_t>;

#define CONST_RAW_EXEC_ARGS                                                                        \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const func_map *avail_funcs

#define _N_EXEC		  exec
#define _N_EXEC_UTILS _N_EXEC::utils

namespace _N_EXEC {
#define RAW_EXEC_ARGS                                                                              \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *mem_space,       \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] func_map *avail_funcs
#define FWD_RAW_ARGS instr, mem_space, frame, avail_funcs
using ret_t = std::ptrdiff_t;
using func_t = ret_t(RAW_EXEC_ARGS);

}; // namespace _N_EXEC
