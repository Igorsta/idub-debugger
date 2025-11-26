#pragma once

#include "../core/exec.hpp"
#include "../core/debug_data.hpp"
#include "../core/thread_builder.hpp"
#include "../core/thread.hpp"
#include "../config/args.hpp"
#include <regex>


namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using func_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE



namespace _N_PARSE_UTILS {
void nop(PARSE_OPCODE_ARGS);

constexpr std::string regex_func() {
	return "\\s*fun\\s+" + regex_arg_t(operand_t::FUNC_NAME) + "\\s*\\(\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*\\)" + "\\s*->\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*:\\s*" + "\\s*\\{([^}]*)\\}";
}

std::regex make_opcode_pattern(std::string op_name);

void parse_line(const std::string &line, thread_builder_t &builder);

std::tuple<thread_t, thread_dbg_data_t> file_parse(std::string &content);

} // namespace _N_PARSE_UTILS


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
