#pragma once

#include "exec.hpp"

struct thread_dbg_data_t;

#define _N_PRNT		  prnt
#define _N_PRNT_UTILS _N_PARSE::utils

namespace _N_PRNT {
#define PRNT_OPCODE_ARGS                                                                           \
	[[maybe_unused]] const instrutction_t *instr, const frame_t *frame, const thread_dbg_data_t &dbg

using func_t = void(PRNT_OPCODE_ARGS);
}; // namespace _N_PRNT
