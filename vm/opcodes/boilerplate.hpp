#pragma once

#define PARSE_SIMPLE_IMPL(macro)                                                                   \
	void _N_PARSE::macro(PARSE_OPCODE_ARGS) {                                                      \
		instrutction_t instr{&::_N_EXEC::macro};                                                   \
                                                                                                   \
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			instr.arg[i] = _N_PARSE_UTILS::parse_arg_t(builder, args[i], matches[i + 1]);          \
		}                                                                                          \
		builder.func_builder.add_instr(instr);                                                     \
	}


#define PARSE_JUMPS_IMPL(macro)                                                                    \
	void _N_PARSE::macro(PARSE_OPCODE_ARGS) {                                                      \
		instrutction_t instr{&::_N_EXEC::macro};                                                   \
                                                                                                   \
		int other_idx = 1;                                                                         \
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			if (args[i] == operand_t::LABEL_ID) {                                                  \
				instr.arg[0] =                                                                     \
					_N_PARSE_UTILS::parse_arg_t(builder, operand_t::LABEL_ID, matches[i + 1]);     \
			} else {                                                                               \
				instr.arg[other_idx++] =                                                           \
					_N_PARSE_UTILS::parse_arg_t(builder, args[i], matches[i + 1]);                 \
			}                                                                                      \
		}                                                                                          \
                                                                                                   \
		builder.func_builder.add_jump(instr);                                                      \
	}

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


#define EXEC_ARITH_IN_PLACE_IMPL(macro, oper)                                                      \
	_N_EXEC::ret_t _N_EXEC::macro(RAW_EXEC_ARGS) {                                                 \
		using namespace _N_EXEC_UTILS;                                                             \
                                                                                                   \
		auto arg0 = instr->arg[0];                                                                 \
		auto arg1 = instr->arg[1];                                                                 \
		auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);                                                  \
		auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);                                                  \
                                                                                                   \
		reg0 oper reg1;                                                                            \
		return 1;                                                                                  \
	}
