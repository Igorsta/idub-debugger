#include "inline.h"
#include "musttail.h"
#include <bits/stdc++.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

///////////////////// DECLARATIONS ////////////////////////////

struct instrutction_t;
struct frame_t;
struct memory_space;
struct thread_t;
struct thread_builder_t;
struct function_t;
enum class operand_t;

///////////////////// EXEC MACRO ////////////////////////////

#define EXEC(instr, mem_space, frame, program) instr->action(instr, mem_space, frame, program)

#define CONST_OPCODE_ARGS                                                                          \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const thread_t *exec_program
#define FRWARD_ARGS instr, mem_space, frame, exec_program

#define EXEC_NEXT(step)                                                                            \
	instr += step;                                                                                 \
	MUST_TAIL return EXEC(instr, mem_space, frame, exec_program)

#define EXEC_START(instr, mem_space, frame, program) EXEC(instr, mem_space, frame, program)

#define _N_EXEC exec
#define _N_EXEC_RAW _N_EXEC::raw
#define _N_EXEC_FULL _N_EXEC::full
#define _N_EXEC_UTILS _N_EXEC::utils

namespace _N_EXEC_FULL {
#define OPCODE_ARGS                                                                                \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *&mem_space,      \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] thread_t *exec_program

using Fun = void(OPCODE_ARGS);

}; // namespace _N_EXEC_FULL

namespace _N_EXEC_RAW {
#define REF_OPCODE_ARGS                                                                            \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *&mem_space,      \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] thread_t *exec_program

using RawFun = void(REF_OPCODE_ARGS);

}; // namespace _N_EXEC_RAW

#define _N_PARSE parse
#define _N_PARSE_OTHER _N_PARSE::other
#define _N_PARSE_SIMPLE _N_PARSE::simple
#define _N_PARSE_UTILS _N_PARSE::utils

namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using parse_instr_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE

using bit64 = uint64_t;
using code_block = std::vector<instrutction_t>;

using func_id_t = bit64;
using labl_id_t = bit64;

using labl_id_map = std::unordered_map<std::string, labl_id_t>;
using labl_map = std::unordered_map<func_id_t, size_t>;

using func_id_map = std::unordered_map<std::string, func_id_t>;
using func_map = std::unordered_map<func_id_t, function_t>;

///////////////////// IMPL ////////////////////////////

struct frame_t {
	const instrutction_t *instr;
	uint64_t *stack_start;
	uint64_t *stack_head;
};

struct function_t {
	code_block body;
	uint64_t no_of_args;
};

struct memory_space {
	const size_t MEM_STACK_SIZE;
	const size_t REGISTERS_SIZE;
	const size_t CALL_STACK_SIZE;

	std::unique_ptr<uint64_t[]> MEM_STACK;
	std::unique_ptr<uint64_t[]> REGISTERS;
	std::unique_ptr<frame_t[]> CALL_STACK;

	memory_space(size_t MEM_STACK_SIZE = 0, size_t REGISTERS_SIZE = 0, size_t CALL_STACK_SIZE = 0) :
		MEM_STACK_SIZE(MEM_STACK_SIZE),
		REGISTERS_SIZE(REGISTERS_SIZE),
		CALL_STACK_SIZE(CALL_STACK_SIZE),
		MEM_STACK(std::make_unique<uint64_t[]>(MEM_STACK_SIZE)),
		REGISTERS(std::make_unique<uint64_t[]>(REGISTERS_SIZE)),
		CALL_STACK(std::make_unique<frame_t[]>(CALL_STACK_SIZE)) {
		assert(MEM_STACK && REGISTERS && CALL_STACK);
	}
};

constexpr size_t args_per_instr = 2;

struct instrutction_t {
	_N_EXEC_FULL::Fun *action;
	bit64 arg[args_per_instr];
};
struct thread_t {
	func_map functions;
	func_id_map func_id;
	memory_space memory;

	void start_execution() {
		int main_id = func_id.at("main");
		const instrutction_t *instr = functions.at(main_id).body.data();
		auto frame = memory.CALL_STACK.get();
		auto mem = &memory;

		frame->stack_head = memory.MEM_STACK.get();
		frame->stack_start = memory.MEM_STACK.get();

		EXEC_START(instr, mem, frame, this);
	}
};

struct thread_builder_t {
public:
	func_map functions_impl;
	func_id_map func_decl;

	labl_id_map labl_decl;
	labl_map label_pos;

	std::vector<size_t> jump_ops{};

	func_id_t built_func;
	bool is_building = false;

	void add_instr(instrutction_t &instr) {
		assert(is_building);
		functions_impl[built_func].body.push_back(instr);
	}

	void add_jump(instrutction_t &instr) {
		assert(is_building);
		jump_ops.push_back(functions_impl[built_func].body.size());
		functions_impl[built_func].body.push_back(instr);
	}

	void define_func(std::string name, uint64_t no_of_args) {
		assert(is_building == false);
		is_building = true;
		built_func = refer_func(name);
		functions_impl[built_func];
		functions_impl[built_func].no_of_args = no_of_args;
	}

	void define_labl(std::string name) {
		assert(is_building);
		label_pos[refer_label(name)] = functions_impl[built_func].body.size();
	}

	void commit_func() {
		if (is_building == false) {
			return;
		}

		for (const auto &[name, id] : labl_decl) {
			assert(label_pos.find(id) != label_pos.end());
		}
		while (jump_ops.size()) {
			int curr_pos = jump_ops.back();
			jump_ops.pop_back();
			instrutction_t &jmp_instr = functions_impl[built_func].body[curr_pos];
			labl_id_t label_id = jmp_instr.arg[0];
			jmp_instr.arg[0] = label_pos[label_id] - curr_pos;
		}
		is_building = false;
	};

	thread_t make_thread() {
		return {
			.functions = functions_impl,
			.func_id = func_decl,
			.memory = memory_space(10, 10, 100),
		};
	}

	func_id_t refer_func(const std::string &input) {
		assert(is_building);
		return func_decl.emplace(input, func_decl.size()).first->second;
	}

	labl_id_t refer_label(const std::string &input) {
		assert(is_building);
		return labl_decl.emplace(input, labl_decl.size()).first->second;
	}

	void validate_prog() {
		for (auto &[_, id] : func_decl) {
			assert(functions_impl.find(id) != functions_impl.end());
		}
	}
};

///////////////////// OPCODE_MACROS ////////////////////////////
#define STCK_INIT init
#define STCK_DEINIT deinit
#define REG_TO_STCK reg_to_stack
#define STCK_TO_REG stack_to_reg
#define REG_TO_RVAL reg_to_retval
#define REG_TO_REG reg_to_reg
#define INPUT_TO_REG input_reg
#define OUTPUT_REG output_reg
#define FUNC_RET func_return
#define EXIT_PROG exit_prog
#define CALL call_func
#define LABEL label
#define JUMP jump_to

#define _N_ARGS args
#define _N_ARGS_UTILS _N_ARGS::utils

#define STRINGIFY(x) #x
#define nameof(x) STRINGIFY(x)

#define REQUIRED_FOR_SIMPLE(macro)                                                                 \
	namespace _N_EXEC_RAW {                                                                        \
	INLINE RawFun macro;                                                                           \
	}                                                                                              \
	namespace _N_EXEC_FULL {                                                                       \
	Fun macro;                                                                                     \
	}                                                                                              \
	namespace _N_ARGS {                                                                            \
	const std::vector<operand_t> &macro();                                                         \
	}                                                                                              \
	namespace _N_PARSE_SIMPLE {                                                                    \
	parse_instr_t macro;                                                                           \
	}                                                                                              \
	namespace _N_EXEC_UTILS::next_instr_offset {                                                   \
	constexpr size_t macro();                                                                      \
	}

namespace _N_PARSE_OTHER {
parse_instr_t LABEL;
}

REQUIRED_FOR_SIMPLE(STCK_INIT)
REQUIRED_FOR_SIMPLE(STCK_DEINIT)
REQUIRED_FOR_SIMPLE(REG_TO_STCK)
REQUIRED_FOR_SIMPLE(STCK_TO_REG)
REQUIRED_FOR_SIMPLE(REG_TO_RVAL)
REQUIRED_FOR_SIMPLE(REG_TO_REG)
REQUIRED_FOR_SIMPLE(INPUT_TO_REG)
REQUIRED_FOR_SIMPLE(OUTPUT_REG)
REQUIRED_FOR_SIMPLE(FUNC_RET)
REQUIRED_FOR_SIMPLE(EXIT_PROG)
REQUIRED_FOR_SIMPLE(CALL)
REQUIRED_FOR_SIMPLE(JUMP)

///////////////////// INSTRUCTION_DEFINITION ////////////////////////////

// STCK_INIT
// STCK_DEINIT
// REG_TO_STCK
// STCK_TO_REG
// REG_TO_RVAL
// REG_TO_REG
// INPUT_TO_REG
// OUTPUT_REG
// FUNC_RET
// EXIT_PROG
// CALL

// struct code_pos {
// 	uint64_t pos;
// 	func_id func;
// };

///////////////////// PARSING INSTRUCTIONS ////////////////////////////

enum class operand_t {
	DECIMAL_NUM,
	REGISTER_ID,
	FUNC_NAME,
	LABEL_ID,
};

constexpr std::string regex_arg_t(const operand_t &arg) {
	switch (arg) {
	case operand_t::DECIMAL_NUM:
		return "(\\d+)";
	case operand_t::REGISTER_ID:
		return "\\$(\\d+)";
	case operand_t::FUNC_NAME:
		return "([A-Za-z0-9_]+)";
	case operand_t::LABEL_ID:
		return "([A-Za-z0-9_]+)";
	default:
		assert(false);
	}
};

constexpr std::string regex_func() {
	return "\\s*fun\\s+" + regex_arg_t(operand_t::FUNC_NAME) + "\\s*\\(\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*\\)\\s*:" + "\\s*\\{([^}]*)\\}";
}

namespace _N_PARSE_UTILS {
constexpr bit64 parse_arg_t(thread_builder_t &builder, operand_t type, std::string input) {
	switch (type) {
	case operand_t::DECIMAL_NUM:
		return std::stoull(input);
	case operand_t::REGISTER_ID:
		return std::stoull(input);
	case operand_t::FUNC_NAME:
		return builder.refer_func(input);
	case operand_t::LABEL_ID:
		return builder.refer_label(input);
	default:
		assert(false);
	}
};
} // namespace _N_PARSE_UTILS

#define ENLIST_OPERANDS(macro, ...)                                                                \
	const std::vector<operand_t> &_N_ARGS::macro() {                                               \
		static const std::vector<operand_t> inner = {__VA_ARGS__};                                 \
		return inner;                                                                              \
	}

ENLIST_OPERANDS(STCK_INIT)
ENLIST_OPERANDS(STCK_DEINIT)
ENLIST_OPERANDS(REG_TO_STCK, operand_t::REGISTER_ID)
ENLIST_OPERANDS(STCK_TO_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(REG_TO_RVAL, operand_t::REGISTER_ID)
ENLIST_OPERANDS(REG_TO_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(INPUT_TO_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(OUTPUT_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(FUNC_RET)
ENLIST_OPERANDS(EXIT_PROG)
ENLIST_OPERANDS(CALL, operand_t::FUNC_NAME)
ENLIST_OPERANDS(JUMP, operand_t::LABEL_ID)

namespace _N_ARGS {
const std::vector<operand_t> &LABEL();
}
ENLIST_OPERANDS(LABEL, operand_t::LABEL_ID)

namespace _N_ARGS_UTILS {
std::unordered_map<std::string, const std::vector<operand_t> &> str_to_arg = {
	{nameof(STCK_INIT), STCK_INIT()},
	{nameof(STCK_DEINIT), STCK_DEINIT()},
	{nameof(REG_TO_STCK), REG_TO_STCK()},
	{nameof(STCK_TO_REG), STCK_TO_REG()},
	{nameof(REG_TO_RVAL), REG_TO_RVAL()},
	{nameof(REG_TO_REG), REG_TO_REG()},
	{nameof(INPUT_TO_REG), INPUT_TO_REG()},
	{nameof(OUTPUT_REG), OUTPUT_REG()},
	{nameof(FUNC_RET), FUNC_RET()},
	{nameof(EXIT_PROG), EXIT_PROG()},
	{nameof(CALL), CALL()},
	{nameof(LABEL), LABEL()},
	{nameof(JUMP), JUMP()},
};

}; // namespace _N_ARGS_UTILS

//////////////////////////// PARSE SIMPLE OPCODE IMPL ////////////////////////////

#define PARSE_SIMPLE_IMPL(macro)                                                                   \
	void _N_PARSE_SIMPLE::macro(PARSE_OPCODE_ARGS) {                                               \
		instrutction_t instr;                                                                      \
		instr.action = &::_N_EXEC_FULL::macro;                                                     \
		for (size_t i = 0; i < args_per_instr; i++) {                                              \
			instr.arg[i] = 0;                                                                      \
		}                                                                                          \
                                                                                                   \
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			instr.arg[i] = _N_PARSE_UTILS::parse_arg_t(builder, args[i], matches[i + 1]);          \
		}                                                                                          \
		builder.add_instr(instr);                                                                  \
	}

PARSE_SIMPLE_IMPL(STCK_INIT)
PARSE_SIMPLE_IMPL(STCK_DEINIT)
PARSE_SIMPLE_IMPL(REG_TO_STCK)
PARSE_SIMPLE_IMPL(STCK_TO_REG)
PARSE_SIMPLE_IMPL(REG_TO_RVAL)
PARSE_SIMPLE_IMPL(REG_TO_REG)
PARSE_SIMPLE_IMPL(INPUT_TO_REG)
PARSE_SIMPLE_IMPL(OUTPUT_REG)
PARSE_SIMPLE_IMPL(FUNC_RET)
PARSE_SIMPLE_IMPL(EXIT_PROG)
PARSE_SIMPLE_IMPL(CALL)

void _N_PARSE_SIMPLE::JUMP(PARSE_OPCODE_ARGS) {
	instrutction_t instr;
	instr.action = &::_N_EXEC_FULL::JUMP;
	for (size_t i = 0; i < args_per_instr; i++) {
		instr.arg[i] = 0;
	}

	instr.arg[0] = _N_PARSE_UTILS::parse_arg_t(builder, operand_t::LABEL_ID, matches[1]);

	builder.add_jump(instr);
}

void _N_PARSE_OTHER::LABEL(PARSE_OPCODE_ARGS) { builder.define_labl(matches[1]); }

namespace _N_PARSE_UTILS {
void nop(PARSE_OPCODE_ARGS) { return; }

std::regex make_opcode_pattern(std::string op_name) {
	std::string ans = "\\s*" + op_name;

	using _N_ARGS_UTILS::str_to_arg;

	auto it = str_to_arg.find(op_name);
	assert(it != str_to_arg.end());
	const auto &args = it->second;

	for (int i = 0; i < args.size(); i++) {
		ans += ("\\s+" + regex_arg_t(args[i]) + ",");
	}
	if (args.size()) {
		ans.pop_back(); // removing last "," if present
	}
	ans += "\\s*(?:#.*)?"; // komentarze są dozwolone

	return std::regex(ans);
}

void parse_line(const std::string &line, thread_builder_t &builder) {

	using namespace ::_N_PARSE_SIMPLE;
	using _N_PARSE_UTILS::make_opcode_pattern;
	using _N_PARSE_UTILS::nop;

	static const std::unordered_map<std::string, std::pair<std::regex, parse_instr_t *>>
		str_to_instr = {
			{nameof(STCK_INIT), {make_opcode_pattern(nameof(STCK_INIT)), &STCK_INIT}},
			{nameof(STCK_DEINIT), {make_opcode_pattern(nameof(STCK_DEINIT)), &STCK_DEINIT}},
			{nameof(REG_TO_STCK), {make_opcode_pattern(nameof(REG_TO_STCK)), &REG_TO_STCK}},
			{nameof(STCK_TO_REG), {make_opcode_pattern(nameof(STCK_TO_REG)), &STCK_TO_REG}},
			{nameof(REG_TO_RVAL), {make_opcode_pattern(nameof(REG_TO_RVAL)), &REG_TO_RVAL}},
			{nameof(REG_TO_REG), {make_opcode_pattern(nameof(REG_TO_REG)), &REG_TO_REG}},
			{nameof(INPUT_TO_REG), {make_opcode_pattern(nameof(INPUT_TO_REG)), &INPUT_TO_REG}},
			{nameof(OUTPUT_REG), {make_opcode_pattern(nameof(OUTPUT_REG)), &OUTPUT_REG}},
			{nameof(EXIT_PROG), {make_opcode_pattern(nameof(EXIT_PROG)), &EXIT_PROG}},
			{nameof(FUNC_RET), {make_opcode_pattern(nameof(FUNC_RET)), &FUNC_RET}},
			{nameof(EXIT_PROG), {make_opcode_pattern(nameof(EXIT_PROG)), &EXIT_PROG}},
			{nameof(CALL), {make_opcode_pattern(nameof(CALL)), &CALL}},
			{nameof(LABEL), {make_opcode_pattern(nameof(LABEL)), &::_N_PARSE_OTHER::LABEL}},
			{nameof(JUMP), {make_opcode_pattern(nameof(JUMP)), &JUMP}},
			{"#", {std::regex("\\s*(?:#.*)?"), nop}},
		};

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
	std::cout << "line: \"" << line << "\"\n";

	assert(false);
	return;
}

thread_t file_parse(std::string &content) {
	thread_builder_t res{};

	static const std::regex func_re(regex_func());
	std::sregex_iterator func_begin(content.begin(), content.end(), func_re), func_end{};

	for (auto it = func_begin; it != func_end; it++) {
		const std::smatch &matches = *it;

		res.define_func(matches[1], stoull(matches[2]));

		std::stringstream code(matches[3]);
		static std::string line;
		while (getline(code, line)) {
			parse_line(line, res);
		}
		res.commit_func();
	}

	res.validate_prog();

	return res.make_thread();
}

} // namespace _N_PARSE_UTILS

std::string read_file(std::string file_name) {
	std::ifstream file(std::filesystem::absolute(file_name));
	std::string content = {std::istreambuf_iterator<char>(file), {}};

	std::regex code_of_functions("(" + regex_func() + ")*\\s*");
	std::smatch matches;

	if (!std::regex_match(content, code_of_functions)) {
		throw std::logic_error("Code of wrong format");
	}

	return content;
}

//////////////////////////// OPCODE RAW ACTION IMPL ////////////////////////////

INLINE static uint64_t *stk_top(CONST_OPCODE_ARGS) {
	assert(frame->stack_start < frame->stack_head);
	return (frame->stack_head - 1);
}

void _N_EXEC_RAW::STCK_INIT(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_head + 1 < end);

	frame->stack_head++;
}

void _N_EXEC_RAW::STCK_DEINIT(REF_OPCODE_ARGS) { frame->stack_head = stk_top(FRWARD_ARGS); }

void _N_EXEC_RAW::REG_TO_STCK(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	*stk_top(FRWARD_ARGS) = mem_space->REGISTERS[arg0];
}

void _N_EXEC_RAW::STCK_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg0] = *stk_top(FRWARD_ARGS);
}

void _N_EXEC_RAW::REG_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];
	assert(arg0 < mem_space->REGISTERS_SIZE);
	assert(arg1 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg1] = mem_space->REGISTERS[arg0];
}

void _N_EXEC_RAW::REG_TO_RVAL(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	*frame->stack_start = mem_space->REGISTERS[arg0];
}

void _N_EXEC_RAW::INPUT_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	size_t val;
	std::cin >> val;

	mem_space->REGISTERS[arg0] = val;
}

void _N_EXEC_RAW::OUTPUT_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	std::cout << mem_space->REGISTERS[arg0] << std::endl;
}

void _N_EXEC_RAW::CALL(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = exec_program->functions.find(arg0);
	assert(it != exec_program->functions.end());

	frame->instr = instr + 1;

	auto prev_frame = frame;
	auto end = mem_space->CALL_STACK.get() + mem_space->CALL_STACK_SIZE;
	assert(frame + 1 < end);
	frame++;

	frame->stack_head = prev_frame->stack_head;

	auto &[_, func_code] = *it;

	auto shared_memory = 1 + func_code.no_of_args;
	assert(frame->stack_start + shared_memory <= frame->stack_head);
	frame->stack_start = frame->stack_head - shared_memory;

	instr = func_code.body.data();
}

void _N_EXEC_RAW::FUNC_RET(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_start + 1 < end);

	frame->stack_head = frame->stack_start + 1;
	frame--;
	instr = frame->instr;
}

void _N_EXEC_RAW::EXIT_PROG(REF_OPCODE_ARGS) {
	assert(frame->stack_head == mem_space->MEM_STACK.get() + 1);
	assert(frame == mem_space->CALL_STACK.get());

	std::cout << std::format("program exited with {}\n", *mem_space->MEM_STACK.get());
}

void _N_EXEC_RAW::JUMP(REF_OPCODE_ARGS) { instr += instr->arg[0]; }

//////////////////////////// OPCODE OFFSET IMPL ////////////////////////////

#define INSTR_OFFSET_IMPL(macro, offset)                                                           \
	namespace _N_EXEC_UTILS::next_instr_offset {                                                   \
	constexpr size_t macro() { return offset; }                                                    \
	}

INSTR_OFFSET_IMPL(STCK_INIT, 1)
INSTR_OFFSET_IMPL(STCK_DEINIT, 1)
INSTR_OFFSET_IMPL(REG_TO_STCK, 1)
INSTR_OFFSET_IMPL(STCK_TO_REG, 1)
INSTR_OFFSET_IMPL(REG_TO_RVAL, 1)
INSTR_OFFSET_IMPL(REG_TO_REG, 1)
INSTR_OFFSET_IMPL(INPUT_TO_REG, 1)
INSTR_OFFSET_IMPL(OUTPUT_REG, 1)
INSTR_OFFSET_IMPL(FUNC_RET, 0)
INSTR_OFFSET_IMPL(CALL, 0)
INSTR_OFFSET_IMPL(JUMP, 0)

//////////////////////////// OPCODE FULL EXEC IMPL ////////////////////////////

#define EXEC_FULL_IMPL(macro)                                                                      \
	void _N_EXEC_FULL::macro(OPCODE_ARGS) {                                                        \
		_N_EXEC_RAW::macro(FRWARD_ARGS);                                                           \
                                                                                                   \
		EXEC_NEXT(_N_EXEC_UTILS::next_instr_offset::macro());                                      \
	}

EXEC_FULL_IMPL(STCK_INIT)
EXEC_FULL_IMPL(STCK_DEINIT)
EXEC_FULL_IMPL(REG_TO_STCK)
EXEC_FULL_IMPL(STCK_TO_REG)
EXEC_FULL_IMPL(REG_TO_RVAL)
EXEC_FULL_IMPL(REG_TO_REG)
EXEC_FULL_IMPL(INPUT_TO_REG)
EXEC_FULL_IMPL(OUTPUT_REG)
EXEC_FULL_IMPL(FUNC_RET)
EXEC_FULL_IMPL(CALL)
EXEC_FULL_IMPL(JUMP)

void _N_EXEC_FULL::EXIT_PROG(OPCODE_ARGS) {
	_N_EXEC_RAW::EXIT_PROG(FRWARD_ARGS);

	return;
}

//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		throw std::runtime_error("the usage: " + std::string(argv[0]) + " <file name>");
	}

	std::string file = argv[1];
	std::string file_content = read_file(file);

	thread_t program = _N_PARSE_UTILS::file_parse(file_content);

	program.start_execution();

	return 0;
}
