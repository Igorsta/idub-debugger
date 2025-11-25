#include "args.hpp"
#include "biject_map.hpp"
#include "config.hpp"
#include "debug.h"
#include "defer.h"
#include "exec.hpp"
#include "inline.h"
#include "io_handler.hpp"
#include "memory.hpp"
#include "opcode_def.hpp"
#include "parse.hpp"
#include "prnt.hpp"
#include "thread.hpp"
#include "debug_data.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

///////////////////// DECLARATIONS ////////////////////////////

struct func_builder_t;
struct thread_builder_t;

///////////////////// USINGS ////////////////////////////


#define STRINGIFY(x) #x
#define nameof(x)	 STRINGIFY(x)

#define REQUIRED_FOR_EXEC(macro)                                                                   \
	REQUIRE_PARSE(macro)                                                                           \
	namespace _N_EXEC {                                                                            \
	INLINE func_t macro;                                                                           \
	}                                                                                              \
	namespace _N_PRNT {                                                                            \
	INLINE func_t macro;                                                                           \
	}

APPLY_TO_EXEC(REQUIRED_FOR_EXEC)
APPLY_TO_OTHER(REQUIRE_PARSE)


namespace _N_PRNT_UTILS {
std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> const &dict() {
	static std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> inner = {
		#define PRNT_ENTRY(macro) {&_N_EXEC::macro, &_N_PRNT::macro},
	APPLY_TO_SIMPLE(PRNT_ENTRY) APPLY_TO_JUMP(PRNT_ENTRY)
	};
	
	return inner;
}

} // namespace _N_PRNT_UTILS

struct func_builder_t {
	func_id_map &func_decl;

	labl_id_map labl_decl;
	labl_map label_pos;

	reg_id_map reg_decl;

	std::vector<size_t> jump_ops;

	code_block built_func_body;

	void add_instr(instrutction_t &instr) { built_func_body.push_back(instr); }

	void add_jump(instrutction_t &instr) {
		jump_ops.push_back(built_func_body.size());
		add_instr(instr);
	}

	void define_labl(const std::string &name) {
		label_pos[refer_label(name)] = built_func_body.size();
	}

	void define_reg(const std::string &name) {
		CORE_ASSERT(reg_decl.emplace_first(name, reg_decl.size()).first,
					"The register {} has been defined two times", name);
	}

	func_id_t refer_func(const std::string &input) {
		return func_decl.emplace_first(input, func_decl.size()).second;
	}

	labl_id_t refer_label(const std::string &input) {
		return labl_decl.emplace_first(input, labl_decl.size()).second;
	}

	reg_id_t refer_reg(const std::string &input) {
		auto held = reg_decl.by_first(input);
		CORE_ASSERT(held.has_value(), "Using undeclared register {}", input);
		return held.value();
	}

	thread_dbg_data_t::func_dbg_data_t make_debug_data() {

		std::vector<_N_EXEC::func_t *> type_of_instr;

		for (int i = 0; i < built_func_body.size(); i++) {
			type_of_instr.push_back(built_func_body[i].action);
		}

		return {
			.registers = reg_decl,
			.labels = labl_decl,
			.label_positions = label_pos,
			.instr_type = type_of_instr,
		};
	}

	auto invoke_commit(const std::string &name, uint64_t no_of_args, uint64_t size_of_result) {
		for (const auto &[name, id] : labl_decl.map_1_to_2) {
			CORE_ASSERT(label_pos.find(id) != label_pos.end(),
						"label {} was refered but not defined", name);
		}
		while (jump_ops.size()) {
			int curr_pos = jump_ops.back();
			jump_ops.pop_back();
			instrutction_t &jmp_instr = built_func_body[curr_pos];
			labl_id_t label_id = jmp_instr.arg[0];
			jmp_instr.arg[0] = label_pos[label_id] - curr_pos;
		}

		defer(built_func_body.clear(); reg_decl.clear(); label_pos.clear(); labl_decl.clear(););

		return std::make_tuple(refer_func(name),
							   function_t{
								   .body = built_func_body,
								   .no_of_args = no_of_args,
								   .res_size = size_of_result,
								   .regs = std::vector<unit>(reg_decl.size()),
							   },
							   make_debug_data());
	}

	func_builder_t(func_id_map &ref) :
		func_decl(ref) {
			CORE_ASSERT(is_empty(), "Initialization of func_builder got messed up")
		};

	bool is_empty() {
		return built_func_body.empty() && reg_decl.empty() && label_pos.empty() &&
			   labl_decl.empty() && jump_ops.empty();
	}
};

struct thread_builder_t {
public:
	static constexpr std::string start_func_name = "main";
	func_map functions_impl;
	func_id_map func_decl;
	thread_dbg_data_t debug_data;
	func_builder_t func_builder = func_builder_t(func_decl);

	void commit_func(const std::string &name, uint64_t no_of_args, uint64_t size_of_result) {
		auto [id, impl, debug] = func_builder.invoke_commit(name, no_of_args, size_of_result);

		functions_impl.emplace(id, impl);
		debug_data.func_data.emplace(id, debug);
		CORE_ASSERT(debug_data.function_names.emplace_first(name, id).first);
	};

	thread_t make_thread() {
		CORE_ASSERT(func_builder.is_empty(),
					"Trying to make a thread without having a full-built func");
		auto el = func_builder.func_decl.by_first(start_func_name);
		CORE_ASSERT(el.has_value(), "There is no {} in the loaded file", start_func_name);
		return {
			.functions = functions_impl,
			.main_id = el.value(),
			.memory = memory_space(),
		};
	}

	void validate_prog() {
		for (auto &[_, id] : func_decl.map_1_to_2) {
			CORE_ASSERT(functions_impl.find(id) != functions_impl.end(),
						"function {} does not have implementation", _);
		}
	}
};

///////////////////// PARSING INSTRUCTIONS ////////////////////////////

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
};
} // namespace _N_PARSE_UTILS

constexpr std::string regex_func() {
	using _N_PARSE_UTILS::regex_arg_t;
	return "\\s*fun\\s+" + regex_arg_t(operand_t::FUNC_NAME) + "\\s*\\(\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*\\)" + "\\s*->\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*:\\s*" + "\\s*\\{([^}]*)\\}";
}

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
};
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

ENLIST_OPERANDS(STCK_INIT)
ENLIST_OPERANDS(STCK_DEINIT)

ENLIST_OPERANDS(REG_TO_STCK, operand_t::REGISTER_ID)
ENLIST_OPERANDS(STCK_TO_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(REG_TO_RVAL, operand_t::REGISTER_ID)
ENLIST_OPERANDS(REG_FROM_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)

ENLIST_OPERANDS(INPUT_TO_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(OUTPUT_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(INIT_IMM, operand_t::REGISTER_ID, operand_t::DECIMAL_NUM)

ENLIST_OPERANDS(CMP_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(FUNC_RET)
ENLIST_OPERANDS(EXIT_PROG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(CALL, operand_t::FUNC_NAME)

ENLIST_OPERANDS(JUMP, operand_t::LABEL_ID)
ENLIST_OPERANDS(JUMP_EQ, operand_t::LABEL_ID)

ENLIST_OPERANDS(LABEL, operand_t::LABEL_ID)
ENLIST_OPERANDS(DEMAND_REG, operand_t::REGISTER_ID)

ENLIST_OPERANDS(ALLOC_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DEALLOC_HEAP, operand_t::REG_WITH_PTR)
ENLIST_OPERANDS(READ_HEAP, operand_t::REGISTER_ID, operand_t::REG_WITH_PTR)
ENLIST_OPERANDS(WRITE_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)

ENLIST_OPERANDS(ADD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MUL_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DIV_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(SUB_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MOD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)

#define NAME_BIND(code) {nameof(code), code()},

namespace _N_ARGS_UTILS {
std::unordered_map<std::string, const std::vector<operand_t> &> str_to_arg = {
	APPLY_TO_ALL(NAME_BIND)};

}; // namespace _N_ARGS_UTILS

//////////////////////////// PARSE SIMPLE OPCODE IMPL ////////////////////////////

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

APPLY_TO_SIMPLE(PARSE_SIMPLE_IMPL)

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

APPLY_TO_JUMP(PARSE_JUMPS_IMPL)

void _N_PARSE::LABEL(PARSE_OPCODE_ARGS) { builder.func_builder.define_labl(matches[1]); }
void _N_PARSE::DEMAND_REG(PARSE_OPCODE_ARGS) { builder.func_builder.define_reg(matches[1]); }

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

APPLY_TO_EXEC(PRNT_IMPL)
namespace _N_PARSE_UTILS {
void nop(PARSE_OPCODE_ARGS) { return; }

std::regex make_opcode_pattern(std::string op_name) {
	std::string ans = "\\s*" + op_name;

	using _N_ARGS_UTILS::str_to_arg;

	auto it = str_to_arg.find(op_name);
	CORE_ASSERT(it != str_to_arg.end(), "{} does not have defined operations", op_name);
	const auto &args = it->second;

	for (int i = 0; i < args.size(); i++) {
		ans += ("\\s+" + regex_arg_t(args[i]) + ",");
	}
	if (args.size()) {
		ans.pop_back(); // removing last "," if present
	}
	ans += "\\s*;";
	ans += "\\s*(?:#.*)?"; // komentarze są dozwolone

	return std::regex(ans);
}

void parse_line(const std::string &line, thread_builder_t &builder) {

	using namespace ::_N_PARSE;
	using _N_PARSE_UTILS::make_opcode_pattern;
	using _N_PARSE_UTILS::nop;

	static const std::unordered_map<std::string, std::pair<std::regex, func_t *>> str_to_instr = {
		{"#", {std::regex("\\s*(?:#.*)?"), nop}},
		{";", {std::regex("\\s*;\\s*(?:#.*)?"), nop}},

#define parse_pos(macro) {nameof(macro), {make_opcode_pattern(nameof(macro)), &macro}},

		APPLY_TO_ALL(parse_pos)};

	std::stringstream input(line);
	std::string prefix;
	input >> prefix;
	std::smatch matches;

	auto quick = str_to_instr.find(prefix);

	if (quick != str_to_instr.end() && std::regex_match(line, matches, quick->second.first)) {
		quick->second.second(builder, matches);
		return;
	}

	for (const auto &[_, entry] : str_to_instr) {
		const auto &[pattern, action] = entry;

		if (std::regex_match(line, matches, pattern)) {
			action(builder, matches);
			return;
		}
	}

	CORE_ASSERT(false, "Line \"{}\" could not be parsed", line);
}

auto file_parse(std::string &content) {
	thread_builder_t res{};

	static const std::regex func_re(regex_func());
	std::sregex_iterator func_begin(content.begin(), content.end(), func_re), func_end{};

	for (auto it = func_begin; it != func_end; it++) {
		const std::smatch &matches = *it;

		std::stringstream code(matches[4]);
		static std::string line;
		while (getline(code, line)) {
			parse_line(line, res);
		}
		res.commit_func(matches[1], stoull(matches[2]), stoull(matches[3]));
	}

	res.validate_prog();

	return std::make_tuple(res.make_thread(), res.debug_data);
}

} // namespace _N_PARSE_UTILS

std::string read_file(std::string file_name) {
	std::ifstream file(std::filesystem::absolute(file_name));
	std::string content = {std::istreambuf_iterator<char>(file), {}};

	std::regex code_of_functions("(" + regex_func() + ")*\\s*");
	std::smatch matches;

	bool matched = std::regex_match(content, code_of_functions);
	CORE_ASSERT(matched, "Code of wrong format");

	return content;
}

//////////////////////////// OPCODE RAW ACTION IMPL ////////////////////////////

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

_N_EXEC::ret_t _N_EXEC::STCK_INIT(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	CORE_ASSERT(frame->stack_head + 1 < end, "Stack overflow");

	frame->stack_head++;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::STCK_DEINIT(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;
	frame->stack_head = last_on_stck(FWD_RAW_ARGS);

	return 1;
}

_N_EXEC::ret_t _N_EXEC::REG_TO_STCK(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;
	auto arg0 = instr->arg[0];

	auto &stack_top = *last_on_stck(FWD_RAW_ARGS);
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	stack_top = reg;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::STCK_TO_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &stack_top = *last_on_stck(FWD_RAW_ARGS);
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	reg = stack_top;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::REG_FROM_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	reg0 = reg1;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::REG_TO_RVAL(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &stack_bottom = *frame->stack_start;
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);
	;

	stack_bottom = reg;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::INPUT_TO_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	size_t val;
	(mem_space->IO) >> val;

	reg = val;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::OUTPUT_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	std::cout << reg << std::endl;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::CALL(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = avail_funcs->find(arg0);
	CORE_ASSERT(it != avail_funcs->end(), "Call to undeclared function");

	frame->instr = instr;

	auto prev_frame = frame;
	auto frame_end = mem_space->CALL_STACK.get() + mem_space->CALL_STACK_SIZE;
	CORE_ASSERT(frame + 1 < frame_end, "Call stack overflow");
	frame++;

	frame->stack_head = prev_frame->stack_head;

	auto &[_, callee] = *it;

	auto shared_memory = callee.no_of_args;
	CORE_ASSERT(prev_frame->stack_start + shared_memory <= prev_frame->stack_head,
				"I guess check number of elements on stack?"); //@todo: WTH is going on here?
	frame->stack_start = frame->stack_head - shared_memory;

	instr = callee.body.data();

	return 0;
}

_N_EXEC::ret_t _N_EXEC::FUNC_RET(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	auto &func = (*avail_funcs)[frame->cur_func_id];

	auto expected_head = frame->stack_start + func.res_size;
	CORE_ASSERT(expected_head == frame->stack_head,
				"Function returned wrong number of elements on a stack");

	frame--;
	frame->stack_head = expected_head;
	instr = frame->instr;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::EXIT_PROG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto ret_val = get_reg(arg0, FWD_RAW_ARGS);

	throw ret_val;
}

_N_EXEC::ret_t _N_EXEC::JUMP(RAW_EXEC_ARGS) { return instr->arg[0]; }

_N_EXEC::ret_t _N_EXEC::CMP_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto &arg0 = instr->arg[0];
	auto &arg1 = instr->arg[1];

	const auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	const auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	frame->flags.were_equal = (reg0 == reg1);
	frame->flags.first_was_bigger = (reg0 > reg1);

	return 1;
}

_N_EXEC::ret_t _N_EXEC::JUMP_EQ(RAW_EXEC_ARGS) {
	return (frame->flags.were_equal) ? instr->arg[0] : 1;
}

_N_EXEC::ret_t _N_EXEC::INIT_IMM(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	reg = arg1;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::ALLOC_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	auto ptr = mem_space->HEAP.allocate(reg1);
	reg0 = ptr;

	return 1;
}

_N_EXEC::ret_t _N_EXEC::DEALLOC_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto reg = get_reg(arg0, FWD_RAW_ARGS);

	mem_space->HEAP.deallocate(reg);

	return 1;
}

_N_EXEC::ret_t _N_EXEC::WRITE_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	CORE_ASSERT(mem_space->HEAP.write(reg0, reg1), "Unsuccessful write to heap");

	return 1;
}

_N_EXEC::ret_t _N_EXEC::READ_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	auto opt = mem_space->HEAP.read(reg1);
	CORE_ASSERT(opt.has_value(), "Accessing a forbidden part of heap {}", reg1);
	reg0 = opt.value();

	return 1;
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

EXEC_ARITH_IN_PLACE_IMPL(ADD_IN_PLACE, +=)
EXEC_ARITH_IN_PLACE_IMPL(MUL_IN_PLACE, *=)
EXEC_ARITH_IN_PLACE_IMPL(SUB_IN_PLACE, -=)
EXEC_ARITH_IN_PLACE_IMPL(DIV_IN_PLACE, /=)
EXEC_ARITH_IN_PLACE_IMPL(MOD_IN_PLACE, %=)

//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

int main(int argc, const char *argv[]) {
	CORE_ASSERT(argc == 3, "the usage: {} [run|debug] <file name>", argv[0]);

	std::string mode = argv[1];
	std::string file = argv[2];
	std::string file_content = read_file(file);

	CORE_ASSERT(mode == "run" || mode == "debug", "Received an unknown command {}", mode);
	auto [program, debug_data] = _N_PARSE_UTILS::file_parse(file_content);

	if (mode == "run") {
		program.start_execution();
	} else {
		debug_data.start_debug_session(std::move(program));
	}

	return 0;
}
