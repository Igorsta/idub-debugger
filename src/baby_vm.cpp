#include <bits/stdc++.h>
#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unordered_map>

///////////////////// DECLARATIONS ////////////////////////////

struct instrutction_t;
struct frame_t;
struct memory_space;
struct program_t;
struct function_t;

///////////////////// MACRO MAGIC ////////////////////////////

#define EXEC(instr, mem_space, frame) instr->action(instr, mem_space, frame)

#define OPCODE_ARGS [[maybe_unused]] const instrutction_t*& instr, [[maybe_unused]] memory_space*& mem_space, [[maybe_unused]] frame_t*& frame
#define REF_OPCODE_ARGS [[maybe_unused]] const instrutction_t*& instr, [[maybe_unused]] memory_space*& mem_space, [[maybe_unused]] frame_t*& frame
#define CONST_OPCODE_ARGS [[maybe_unused]] const instrutction_t* instr, [[maybe_unused]] const memory_space* mem_space, [[maybe_unused]] const frame_t* frame
#define FRWARD_ARGS instr, mem_space, frame

#define EXEC_NEXT(step) instr += step; MUST_TAIL return EXEC(instr, mem_space, frame)
#define EXEC_START(instr, mem_space, frame) EXEC(program, mem_space, frame)

#if defined (__clang__) && __clang__ >= 13
	#define MUST_TAIL [[clang::musttail]]
#elif defined (__GNUG__) && __GNUG__ >= 15
	#define MUST_TAIL [[gnu::musttail]]
#else
	#define MUST_TAIL
	#warning "Using tailcalls without a support from compiler"
#endif

///////////////////// USINGS ////////////////////////////

using Fun = void(OPCODE_ARGS);
using RawFun = void(REF_OPCODE_ARGS);
using bit64 = uint64_t;

using code_block = std::vector<instrutction_t>;

using func_id = bit64;
using label_id = bit64;

using func_id_map = std::unordered_map<std::string, func_id>;
using func_map = std::unordered_map<func_id, function_t>;

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
};

struct memory_space {
	const size_t MEM_STACK_SIZE;
	const size_t REGISTERS_SIZE;
	const size_t CALL_STACK_SIZE;

	std::unique_ptr<uint64_t[]> MEM_STACK;
	std::unique_ptr<uint64_t[]> REGISTERS;
	std::unique_ptr<frame_t[]> CALL_STACK;

	memory_space(size_t MEM_STACK_SIZE = 0, size_t REGISTERS_SIZE = 0, size_t CALL_STACK_SIZE = 0)
		: MEM_STACK_SIZE(MEM_STACK_SIZE),
			REGISTERS_SIZE(REGISTERS_SIZE),
			CALL_STACK_SIZE(CALL_STACK_SIZE),
			MEM_STACK(std::make_unique<uint64_t[]>(MEM_STACK_SIZE)),
			REGISTERS(std::make_unique<uint64_t[]>(REGISTERS_SIZE)),
			CALL_STACK(std::make_unique<frame_t[]>(CALL_STACK_SIZE))
	{
		assert(MEM_STACK && REGISTERS && CALL_STACK);
	}
};

struct program_t {
	func_map functions;
	func_id_map func_id;
	// std::unordered_map<label_id, code_pos> labels;
	memory_space memory;
};

///////////////////// INSTRUCTION_DEFINITION ////////////////////////////

constexpr size_t args_per_instr = 2;

struct instrutction_t {
	Fun *action;
	bit64 arg[args_per_instr];
};

namespace rawFun {
	static RawFun init;
	static RawFun deinit;
	static RawFun reg_to_stk;
	static RawFun stk_to_reg;
	static RawFun reg_to_retval;
	static RawFun reg_to_reg;
	static RawFun input_reg;
	static RawFun output_reg;
	static RawFun ret;
	static RawFun exit;
	static RawFun call;
};

namespace OpFun {
	static Fun init;
	static Fun deinit;
	static Fun reg_to_stk;
	static Fun stk_to_reg;
	static Fun reg_to_retval;
	static Fun reg_to_reg;
	static Fun input_reg;
	static Fun output_reg;
	static Fun ret;
	static Fun exit;
	static Fun call;
};

// struct code_pos {
// 	uint64_t pos;
// 	func_id func;
// };

struct {
	func_id_map functions_id = {{"main", 0}, {"test", 1}}; // temporary change for compilation purpose
	// std::unordered_map<label_id, code_pos> labels;
	// uint64_t curr_pos;
} ctx;

///////////////////// PARSING INSTRUCTIONS ////////////////////////////

enum class arg_t {
	DECIMAL_NUM,
	REGISTER_ID,
	FUNC_NAME,
};

constexpr std::string regex_arg_t(const arg_t& arg) {
	switch (arg) {
		case arg_t::DECIMAL_NUM: return "(\\d+)";
		case arg_t::REGISTER_ID: return "\\$(\\d+)";
		case arg_t::FUNC_NAME: return "([A-Za-z0-9_]+)";
		default:
			assert(false);			
	}
};

constexpr std::string regex_func() {
	return "\\s*fun\\s+" + regex_arg_t(arg_t::FUNC_NAME) + "\\s*:\\s*\\{[^}]*\\}";
}

constexpr bit64 parse_arg_t(func_id_map& functions_id, arg_t type, std::string input) {
	switch (type) {
		case arg_t::DECIMAL_NUM: return std::stoull(input);
		case arg_t::REGISTER_ID: return std::stoull(input);
		case arg_t::FUNC_NAME: return functions_id.emplace(input, functions_id.size()).first->second;
		default:
			assert(false);
	}
};

template <std::same_as<arg_t>... ArgsT>
std::pair<std::regex, std::function<instrutction_t(std::smatch)>> make_opcode(std::string op_name, RawFun* code, ArgsT... args_t) {
	std::string ans = "\\s*" + op_name;

	(ans += ... += ("\\s+" + regex_arg_t(args_t) + ","));
	if (sizeof...(args_t) != 0) {
		ans.pop_back(); 		// removing last "," if present
	}
	ans += "\\s*(?:#.*)?";	// komentarze są dozwolone

	auto converter = [code, args_t...](const std::smatch &match) -> instrutction_t {
		instrutction_t instr;
		instr.action = code;
		for (size_t i = 0; i < args_per_instr; i++) {
			instr.arg[i] = 0;
		}

		int i = 0;
		((instr.arg[i] = parse_arg_t(ctx.functions_id, args_t, match[i + 1]), ++i, true) && ...);

		return instr;
	};

	return {std::regex(ans), converter};
}

std::vector<std::pair<std::regex, std::function<instrutction_t(std::smatch)>>> opcodes = {
    make_opcode("init", &OpFun::init),
    make_opcode("deinit", &OpFun::deinit),
    make_opcode("rts", &OpFun::reg_to_stk, arg_t::REGISTER_ID),
    make_opcode("str", &OpFun::stk_to_reg, arg_t::REGISTER_ID),
    make_opcode("rtrv", &OpFun::reg_to_retval, arg_t::REGISTER_ID),
    make_opcode("rtr", &OpFun::reg_to_reg, arg_t::REGISTER_ID, arg_t::REGISTER_ID),
    make_opcode("ir", &OpFun::input_reg, arg_t::REGISTER_ID),
    make_opcode("or", &OpFun::output_reg, arg_t::REGISTER_ID),
    make_opcode("ret", &OpFun::ret),
    make_opcode("ext", &OpFun::exit),
    make_opcode("call", &OpFun::call, arg_t::FUNC_NAME),
};

instrutction_t make_instr(const std::string& line) {
	std::smatch matches;
	for (auto &[pattern, func] : opcodes) {
		if (std::regex_match(line, matches, pattern)) {
			return func(matches);
		}
	}

	assert(false);
}

static func_map functions = {
	{0,
	 {.body =
		{
		 make_instr("ir $0"),
		 make_instr("init"),
		 make_instr("init"),
		 make_instr("call test"),
		 make_instr("str $1"),
		 make_instr("deinit"),
		 make_instr("or $1"),
		 make_instr("rtrv $0"),
		 make_instr("ext"),
	 },
	 .no_of_args = 0,
	 .registers_begin = 0,
	 .registers_end = 2,
	}},
	{1,
	 {
		 .body =
			 {
				 make_instr("ir $2"),  make_instr("init"),	 make_instr("rts $2"), make_instr("ir $2"),
				 make_instr("init"),   make_instr("rts $2"), make_instr("ir $2"),  make_instr("init"),
				 make_instr("rts $2"), make_instr("str $3"), make_instr("deinit"), make_instr("str $4"),
				 make_instr("deinit"), make_instr("str $5"), make_instr("deinit"), make_instr("or $3"),
				 make_instr("or $4"),  make_instr("or $5"),	 make_instr("ir $2"),  make_instr("rtrv $2"),
				 make_instr("ret"),
			 },
		 .no_of_args = 0,
		 .registers_begin = 2,
		 .registers_end = 6,
		 }},
};

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

program_t file_parse(std::string &content) {
	func_map parsed_functions;

	return {
		.functions = parsed_functions,
		.memory = memory_space(10, 10, 20),
	};
}

void start_execution(program_t& prog) {
	int main_id = prog.func_id.at("main");
	const instrutction_t* instr = prog.functions.at(main_id).body.data();
	auto frame = prog.memory.CALL_STACK.get();
    auto memory = &prog.memory;
	
	frame->stack_head = prog.memory.MEM_STACK.get();
	frame->stack_start = prog.memory.MEM_STACK.get();

	
	instr->action(instr, memory, frame);
}

//////////////////////////// OPCODE ACTION IMPLEMENTATION ////////////////////////////

static inline __attribute__((always_inline))  uint64_t* stk_top(CONST_OPCODE_ARGS) {
	assert(frame->stack_start < frame->stack_head);
	return (frame->stack_head - 1);
}

inline __attribute__((always_inline)) void rawFun::init(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_head + 1 < end);

	frame->stack_head++;
}

inline __attribute__((always_inline)) void rawFun::deinit(REF_OPCODE_ARGS) {
	frame->stack_head = stk_top(FRWARD_ARGS);
}

inline __attribute__((always_inline)) void rawFun::reg_to_stk(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);
    
	*stk_top(FRWARD_ARGS) = mem_space->REGISTERS[arg0];
}

inline __attribute__((always_inline)) void rawFun::stk_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg0] = *stk_top(FRWARD_ARGS);
}

inline __attribute__((always_inline)) void rawFun::reg_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];
	assert(arg0 < mem_space->REGISTERS_SIZE);
	assert(arg1 < mem_space->REGISTERS_SIZE);

	mem_space->REGISTERS[arg1] = mem_space->REGISTERS[arg0];
}

inline __attribute__((always_inline)) void rawFun::reg_to_retval(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);
    
	*frame->stack_start = mem_space->REGISTERS[arg0];
}

inline __attribute__((always_inline)) void rawFun::input_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	size_t val;
	std::cin >> val;

	mem_space->REGISTERS[arg0] = val;
}

inline __attribute__((always_inline)) void rawFun::output_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REGISTERS_SIZE);

	std::cout << mem_space->REGISTERS[arg0] << std::endl;
}

inline __attribute__((always_inline)) void rawFun::call(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = functions.find(arg0);
	assert(it != functions.end());

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

inline __attribute__((always_inline)) void rawFun::ret(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_start + 1 < end);

	frame->stack_head = frame->stack_start + 1;
	frame--;
	instr = frame->instr;
}

inline __attribute__((always_inline)) void rawFun::exit(REF_OPCODE_ARGS) {
  	assert(frame->stack_head == mem_space->MEM_STACK.get() + 1);
	assert(frame == mem_space->CALL_STACK.get());
	
	std::cout << std::format("program exited with {}\n", *mem_space->MEM_STACK.get());
}

//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

void OpFun::init(OPCODE_ARGS) {
	rawFun::init(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::deinit(OPCODE_ARGS) {
	rawFun::deinit(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::stk_to_reg(OPCODE_ARGS) {
	rawFun::stk_to_reg(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::reg_to_stk(OPCODE_ARGS) {
	rawFun::reg_to_stk(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::reg_to_retval(OPCODE_ARGS) {
	rawFun::reg_to_retval(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::reg_to_reg(OPCODE_ARGS) {
	rawFun::reg_to_reg(FRWARD_ARGS);
	
	EXEC_NEXT(1);
}

void OpFun::output_reg(OPCODE_ARGS) {
	rawFun::output_reg(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::input_reg(OPCODE_ARGS) {
	rawFun::input_reg(FRWARD_ARGS);

	EXEC_NEXT(1);
}

void OpFun::call(OPCODE_ARGS) {
	rawFun::call(FRWARD_ARGS);

	EXEC_NEXT(0);
}

void OpFun::ret(OPCODE_ARGS) {
	rawFun::ret(FRWARD_ARGS);

	EXEC_NEXT(0);
}

void OpFun::exit(OPCODE_ARGS) {
	rawFun::exit(FRWARD_ARGS);

	return;
}


//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

memory_space global_space = memory_space(10, 10, 20);

int main(int argc, const char *argv[]) {
	std::cout << argc << "\n";
	if (argc == 2) {
		std::string file = argv[1];
		std::cout << file << "\n";
		std::string file_content = read_file(file);

		file_parse(file_content);

		std::stringstream source(file_content);
		std::string line;
		while (getline(source, line)) {
			std::cout << "\"" << line << "\"\n";
		}
	}

	program_t working_program = {.functions = functions, .func_id = ctx.functions_id, .memory = memory_space(10, 10, 20)};
	
	start_execution(working_program);

	return 0;
}