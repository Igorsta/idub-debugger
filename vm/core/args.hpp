#pragma once

#include "config.hpp"
#include "prnt.hpp"
#include "thread_builder.hpp"


enum class operand_t {
	DECIMAL_NUM,
	REGISTER_ID,
	FUNC_NAME,
	LABEL_ID,
	REG_WITH_PTR,
};


#define ENLIST_OPERANDS(macro, ...)                                                                \
	const std::vector<operand_t> &_N_ARGS::macro() {                                               \
		static const std::vector<operand_t> inner = {__VA_ARGS__};                                 \
		return inner;                                                                              \
	}

	
namespace _N_ARGS_UTILS {
	std::unordered_map<std::string, const std::vector<operand_t> &> const& str_to_arg();
}; // namespace _N_ARGS_UTILS

namespace _N_PARSE_UTILS {
constexpr unit parse_arg_t(thread_builder_t &builder, operand_t type, std::string input) {
	switch (type) {
	case operand_t::DECIMAL_NUM:
		return std::stoull(input);
	case operand_t::REGISTER_ID:
		return builder.func_builder.refer_reg(input);
	case operand_t::REG_WITH_PTR:
		return builder.func_builder.refer_reg(input);
	case operand_t::FUNC_NAME:
		return builder.func_builder.refer_func(input);
	case operand_t::LABEL_ID:
		return builder.func_builder.refer_label(input);
	}

	CORE_ASSERT(false, "operand {} does not have a defined regex", int(type));
}
} // namespace _N_PARSE_UTILS

namespace _N_PARSE_UTILS {
constexpr std::string regex_arg_t(const operand_t &arg) {
	switch (arg) {
	case operand_t::DECIMAL_NUM:
		return "(\\d+)";
	case operand_t::REGISTER_ID:
		return "\\$([A-Za-z0-9_]+)";
	case operand_t::REG_WITH_PTR:
		return "\\[\\$([A-Za-z0-9_]+)\\]";
	case operand_t::FUNC_NAME:
		return "([A-Za-z0-9_]+)";
	case operand_t::LABEL_ID:
		return "([A-Za-z0-9_]+)";
	}

	//@todo make it BEAUTIFUL!
	CORE_ASSERT(false, "operand {} does not have a defined regex", int(arg));
}
} // namespace _N_PARSE_UTILS

namespace _N_PRNT_UTILS {
constexpr std::string print_arg(unit data, func_id_t f, operand_t type,
								const thread_dbg_data_t &dbg) {
	switch (type) {
	case operand_t::DECIMAL_NUM:
		return std::to_string(data);
	case operand_t::REGISTER_ID:
		return "$" + dbg.func_data.at(f).get_reg_name(data);
	case operand_t::REG_WITH_PTR:
		return "[" + print_arg(data, f, operand_t::REGISTER_ID, dbg) + "]";
	case operand_t::FUNC_NAME:
		return dbg.get_func_name(data);
	case operand_t::LABEL_ID:
		return std::to_string(static_cast<int64_t>(data));
	}

	CORE_ASSERT(false, "operand {} does not have a defined prnt func", int(type));
};
} // namespace _N_PRNT_UTILS

