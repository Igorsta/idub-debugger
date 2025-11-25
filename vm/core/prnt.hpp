#pragma once

#include "exec.hpp"

struct thread_dbg_data_t;
struct code_pos_t;
struct thread_t;

#define _N_PRNT		  prnt
#define _N_PRNT_UTILS _N_PARSE::utils

namespace _N_PRNT {
#define PRNT_OPCODE_ARGS                                                                           \
	[[maybe_unused]] code_pos_t code_pos, [[maybe_unused]] thread_t& thr, const thread_dbg_data_t &dbg

using func_t = void(PRNT_OPCODE_ARGS);
}; // namespace _N_PRNT
