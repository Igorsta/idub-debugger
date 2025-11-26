#pragma once

#include "exec.hpp"
#include "debug_data.hpp"
#include "thread.hpp"
#include <iostream>

namespace _N_PRNT {
#define PRNT_OPCODE_ARGS                                                                           \
	[[maybe_unused]] code_pos_t code_pos, [[maybe_unused]] thread_t& thr, const thread_dbg_data_t &dbg

using func_t = void(PRNT_OPCODE_ARGS);
}; // namespace _N_PRNT


namespace _N_PRNT_UTILS {
std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> const& dict();
} // namespace _N_PRNT_UTILS

#define PRNT_IMPL(macro)                                                                           \
	void _N_PRNT::macro(PRNT_OPCODE_ARGS) {                                                        \
		std::cout << nameof(macro);                                                                \
		auto [func_id, pos] = code_pos;                                                            \
		instrutction_t *instr = &thr.functions[func_id].body[pos];                                 \
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			std::cout << " " << _N_PRNT_UTILS::print_arg(instr->arg[i], func_id, args[i], dbg);    \
		}                                                                                          \
		std::cout << "\n";                                                                         \
	}
