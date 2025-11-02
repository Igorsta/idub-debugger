#include "inline.h"
#include "musttail.h"
#include <bits/stdc++.h>
#include <cassert>
#include <cstddef>
#include <unordered_map>

///////////////////// DECLARATIONS ////////////////////////////

struct instrutction_t;
struct frame_t;
struct memory_space;
struct thread_t;
struct function_t;
enum class operand_t;

///////////////////// MACRO MAGIC ////////////////////////////

#define EXEC(instr, mem_space, frame, program) instr->action(instr, mem_space, frame, program)

#define CONST_OPCODE_ARGS                                                                          \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const thread_t *exec_program
#define FRWARD_ARGS instr, mem_space, frame, exec_program

#define EXEC_NEXT(step)                                                                            \
	instr += step;                                                                                 \
	MUST_TAIL return EXEC(instr, mem_space, frame, exec_program)
#define EXEC_START(instr, mem_space, frame, program) EXEC(instr, mem_space, frame, program)

///////////////////// USINGS ////////////////////////////

namespace exec::impl::full {
#define OPCODE_ARGS                                                                                \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *&mem_space,      \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] thread_t *exec_program

using Fun = void(OPCODE_ARGS);

}; // namespace exec::impl::full

namespace exec::impl::raw {
#define REF_OPCODE_ARGS                                                                            \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *&mem_space,      \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] thread_t *exec_program

using RawFun = void(REF_OPCODE_ARGS);

}; // namespace exec::impl::raw

namespace parse_opcode {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_t &, [[maybe_unused]] function_t &, [[maybe_unused]] const std::smatch &

using parse_instr_t = void(PARSE_OPCODE_ARGS);
}; // namespace parse_opcode

using bit64 = uint64_t;
using code_block = std::vector<instrutction_t>;

using func_id_t = bit64;
using label_id = bit64;

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
	uint64_t registers_begin, registers_end;
	// std::unordered_map<label_id, code_pos> labels;
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

struct thread_t {
	func_map functions;
	func_id_map func_id;
	memory_space memory;
};

///////////////////// OPCODE_MACROS ////////////////////////////
#define STCK_INIT init
#define STCK_DEINIT deinit
#define REG_TO_STCK reg_to_stk
#define STCK_TO_REG stk_to_reg
#define REG_TO_RVAL reg_to_retval
#define REG_TO_REG reg_to_reg
#define INPUT_TO_REG input_reg
#define OUTPUT_REG output_reg
#define FUNC_RET ret
#define EXIT_PROG exit
#define CALL call

#define STRINGIFY(x) #x
#define nameof(x) STRINGIFY(x)

#define REQUIRED_FOR_EXEC_INSTR(macro)                                                             \
	namespace exec::impl::raw {                                                                    \
	RawFun macro;                                                                                  \
	}                                                                                              \
	namespace exec::impl::full {                                                                   \
	Fun macro;                                                                                     \
	}                                                                                              \
	namespace instructions::arguments {                                                            \
	const std::vector<operand_t> &macro();                                                         \
	}                                                                                              \
	namespace parse_opcode {                                                                       \
	parse_instr_t macro;                                                                           \
	}                                                                                              \
	namespace exec::impl::next_instr_offset {                                                      \
	constexpr size_t macro();                                                                      \
	}

REQUIRED_FOR_EXEC_INSTR(STCK_INIT)
REQUIRED_FOR_EXEC_INSTR(STCK_DEINIT)
REQUIRED_FOR_EXEC_INSTR(REG_TO_STCK)
REQUIRED_FOR_EXEC_INSTR(STCK_TO_REG)
REQUIRED_FOR_EXEC_INSTR(REG_TO_RVAL)
REQUIRED_FOR_EXEC_INSTR(REG_TO_REG)
REQUIRED_FOR_EXEC_INSTR(INPUT_TO_REG)
REQUIRED_FOR_EXEC_INSTR(OUTPUT_REG)
REQUIRED_FOR_EXEC_INSTR(FUNC_RET)
REQUIRED_FOR_EXEC_INSTR(EXIT_PROG)
REQUIRED_FOR_EXEC_INSTR(CALL)

///////////////////// INSTRUCTION_DEFINITION ////////////////////////////

constexpr size_t args_per_instr = 2;

struct instrutction_t {
	exec::impl::full::Fun *action;
	bit64 arg[args_per_instr];
};

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
};

constexpr std::string regex_arg_t(const operand_t &arg) {
	switch (arg) {
	case operand_t::DECIMAL_NUM:
		return "(\\d+)";
	case operand_t::REGISTER_ID:
		return "\\$(\\d+)";
	case operand_t::FUNC_NAME:
		return "([A-Za-z0-9_]+)";
	default:
		assert(false);
	}
};

constexpr std::string regex_func() {
	return "\\s*fun\\s+" + regex_arg_t(operand_t::FUNC_NAME) + "\\s*\\(\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*\\)\\s*:" + "\\s*\\{([^}]*)\\}";
}

constexpr bit64 parse_arg_t(func_id_map &functions_id, operand_t type, std::string input) {
	switch (type) {
	case operand_t::DECIMAL_NUM:
		return std::stoull(input);
	case operand_t::REGISTER_ID:
		return std::stoull(input);
	case operand_t::FUNC_NAME:
		return functions_id.emplace(input, functions_id.size()).first->second;
	default:
		assert(false);
	}
};

#define ENLIST_OPERANDS(macro, ...)                                                                \
	const std::vector<operand_t> &instructions::arguments::macro() {                               \
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

namespace instructions::arguments {
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
};

}; // namespace instructions::arguments

//////////////////////////// PARSE SIMPLE OPCODE IMPL ////////////////////////////

#define PARSE_OPCODE_IMPL(macro)                                                                   \
	void parse_opcode::macro(thread_t &thread, function_t &built_func,                             \
							 const std::smatch &parsed_input) {                                    \
		instrutction_t instr;                                                                      \
		instr.action = &::exec::impl::full::macro;                                                 \
		for (size_t i = 0; i < args_per_instr; i++) {                                              \
			instr.arg[i] = 0;                                                                      \
		}                                                                                          \
                                                                                                   \
		const auto &args = ::instructions::arguments::macro();                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			instr.arg[i] = parse_arg_t(thread.func_id, args[i], parsed_input[i + 1]);              \
		}                                                                                          \
		built_func.body.push_back(instr);                                                          \
	}

PARSE_OPCODE_IMPL(STCK_INIT)
PARSE_OPCODE_IMPL(STCK_DEINIT)
PARSE_OPCODE_IMPL(REG_TO_STCK)
PARSE_OPCODE_IMPL(STCK_TO_REG)
PARSE_OPCODE_IMPL(REG_TO_RVAL)
PARSE_OPCODE_IMPL(REG_TO_REG)
PARSE_OPCODE_IMPL(INPUT_TO_REG)
PARSE_OPCODE_IMPL(OUTPUT_REG)
PARSE_OPCODE_IMPL(FUNC_RET)
PARSE_OPCODE_IMPL(EXIT_PROG)
PARSE_OPCODE_IMPL(CALL)

namespace parse_opcode {
void nop(thread_t &, function_t &, const std::smatch &) { return; }

std::regex make_opcode_pattern(std::string op_name) {
	std::string ans = "\\s*" + op_name;

	auto it = instructions::arguments::str_to_arg.find(op_name);
	assert(it != instructions::arguments::str_to_arg.end());
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
} // namespace parse_opcode

void parse_line(const std::string &line, thread_t &built_thread, function_t &built_func) {
	using namespace ::parse_opcode;

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
			{nameof(FUNC_RET), {make_opcode_pattern(nameof(FUNC_RET)), &FUNC_RET}},
			{nameof(EXIT_PROG), {make_opcode_pattern(nameof(EXIT_PROG)), &EXIT_PROG}},
			{"#", {std::regex("\\s*(?:#.*)?"), nop}},
		};

	std::stringstream input(line);
	std::string prefix;
	input >> prefix;
	std::smatch matches;

	auto quick = str_to_instr.find(prefix);

	if (quick != str_to_instr.end() && std::regex_match(line, matches, quick->second.first)) {
		quick->second.second(built_thread, built_func, matches);
		return;
	}

	for (const auto &[_, entry] : str_to_instr) {
		const auto &[pattern, action] = entry;

		if (std::regex_match(line, matches, pattern)) {
			action(built_thread, built_func, matches);
			return;
		}
	}

	assert(false);
	return;
}

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

thread_t file_parse(std::string &content) {
	thread_t res{};

	static const std::regex func_re(regex_func());
	std::sregex_iterator func_begin(content.begin(), content.end(), func_re), func_end{};

	for (auto it = func_begin; it != func_end; it++) {
		const std::smatch &matches = *it;

		function_t processed_function = {
			.body = {},
			.no_of_args = stoull(matches[2]),
			.registers_begin = 0,
			.registers_end = 0,
		};
		;

		func_id_t func_id = res.func_id.emplace(matches[1], res.func_id.size()).first->second;

		std::stringstream code(matches[3]);
		static std::string line;
		while (getline(code, line)) {
			parse_line(line, res, processed_function);
		}

		res.functions[func_id] = processed_function;
	}

	return res;
}

void start_execution(thread_t &prog) {
	int main_id = prog.func_id.at("main");
	const instrutction_t *instr = prog.functions.at(main_id).body.data();
	auto frame = prog.memory.CALL_STACK.get();
	auto memory = &prog.memory;

	frame->stack_head = prog.memory.MEM_STACK.get();
	frame->stack_start = prog.memory.MEM_STACK.get();

	EXEC_START(instr, memory, frame, &prog);
}

//////////////////////////// OPCODE RAW ACTION IMPL ////////////////////////////

INLINE static uint64_t *stk_top(CONST_OPCODE_ARGS) {
	assert(frame->stack_start < frame->stack_head);
	return (frame->stack_head - 1);
}

INLINE void exec::impl::raw::init(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_head + 1 < end);

	frame->stack_head++;
}

INLINE void exec::impl::raw::deinit(REF_OPCODE_ARGS) { frame->stack_head = stk_top(FRWARD_ARGS); }

INLINE void exec::impl::raw::reg_to_stk(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	*stk_top(FRWARD_ARGS) = mem_space->REGISTERS[arg0];
}

INLINE void exec::impl::raw::stk_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg0] = *stk_top(FRWARD_ARGS);
}

INLINE void exec::impl::raw::reg_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];
	assert(arg0 < mem_space->REGISTERS_SIZE);
	assert(arg1 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg1] = mem_space->REGISTERS[arg0];
}

INLINE void exec::impl::raw::reg_to_retval(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	*frame->stack_start = mem_space->REGISTERS[arg0];
}

INLINE void exec::impl::raw::input_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	size_t val;
	std::cin >> val;

	mem_space->REGISTERS[arg0] = val;
}

INLINE void exec::impl::raw::output_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	std::cout << mem_space->REGISTERS[arg0] << std::endl;
}

INLINE void exec::impl::raw::call(REF_OPCODE_ARGS) {
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

INLINE void exec::impl::raw::ret(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_start + 1 < end);

	frame->stack_head = frame->stack_start + 1;
	frame--;
	instr = frame->instr;
}

INLINE void exec::impl::raw::exit(REF_OPCODE_ARGS) {
	assert(frame->stack_head == mem_space->MEM_STACK.get() + 1);
	assert(frame == mem_space->CALL_STACK.get());

	std::cout << std::format("program exited with {}\n", *mem_space->MEM_STACK.get());
}

//////////////////////////// OPCODE OFFSET IMPL ////////////////////////////

#define INSTR_OFFSET_IMPL(macro, offset)                                                           \
	namespace exec::impl::next_instr_offset {                                                      \
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

//////////////////////////// OPCODE FULL EXEC IMPL ////////////////////////////

#define INSTR_EXEC_FULL_IMPL(macro)                                                                \
	void exec::impl::full::macro(OPCODE_ARGS) {                                                    \
		exec::impl::raw::macro(FRWARD_ARGS);                                                       \
                                                                                                   \
		EXEC_NEXT(exec::impl::next_instr_offset::macro());                                         \
	}

INSTR_EXEC_FULL_IMPL(STCK_INIT)
INSTR_EXEC_FULL_IMPL(STCK_DEINIT)
INSTR_EXEC_FULL_IMPL(REG_TO_STCK)
INSTR_EXEC_FULL_IMPL(STCK_TO_REG)
INSTR_EXEC_FULL_IMPL(REG_TO_RVAL)
INSTR_EXEC_FULL_IMPL(REG_TO_REG)
INSTR_EXEC_FULL_IMPL(INPUT_TO_REG)
INSTR_EXEC_FULL_IMPL(OUTPUT_REG)
INSTR_EXEC_FULL_IMPL(FUNC_RET)
INSTR_EXEC_FULL_IMPL(CALL)

void exec::impl::full::EXIT_PROG(OPCODE_ARGS) {
	exec::impl::raw::EXIT_PROG(FRWARD_ARGS);

	return;
}

//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		throw std::runtime_error("the usage: " + std::string(argv[0]) + " <file name>");
	}

	std::string file = argv[1];
	std::string file_content = read_file(file);

	thread_t maybe_working_program = file_parse(file_content);

	start_execution(maybe_working_program);

	return 0;
}
