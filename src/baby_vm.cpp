#include <bits/stdc++.h>
#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
using namespace std;

///////////////////// DECLARATIONS ////////////////////////////

struct Instruction;
struct Frame;
struct MemorySpace;
using Code = vector<Instruction>;

///////////////////// DECLRING MEMORY ////////////////////////////

struct Frame {
	const Instruction *instr;
	uint64_t *stack_start;
	uint64_t *stack_head;
};

struct MemorySpace {
	const size_t MEM_SIZE;
	const size_t REG_SIZE;
	const size_t STK_SIZE;

	std::unique_ptr<uint64_t[]> MEM;
	std::unique_ptr<uint64_t[]> REG;
	std::unique_ptr<Frame[]> STK;

	MemorySpace(size_t MEM_SIZE = 0, size_t REG_SIZE = 0, size_t STK_SIZE = 0)
		: MEM_SIZE(MEM_SIZE),
			REG_SIZE(REG_SIZE),
			STK_SIZE(STK_SIZE),
			MEM(std::make_unique<uint64_t[]>(MEM_SIZE)),
			REG(std::make_unique<uint64_t[]>(REG_SIZE)),
			STK(std::make_unique<Frame[]>(STK_SIZE))
	{
		assert(MEM && REG && STK);
	}
};

///////////////////// TAIL-RECURSION ENFORCEMENT ////////////////////////////

#if defined (__clang__) && __clang__ >= 13
	#define MUST_TAIL [[clang::musttail]]
#elif defined (__GNUG__) && __GNUG__ >= 15
	#define MUST_TAIL [[gnu::musttail]]
#else
	#define MUST_TAIL
	#warning "Using tailcalls without a support from compiler"
#endif


///////////////////// INSTRUCTION_HELPERS ////////////////////////////

#define EXEC(instr, mem_space, frame) instr->action(instr, mem_space, frame)

#define OPCODE_ARGS [[maybe_unused]] const Instruction*& instr, [[maybe_unused]] MemorySpace*& mem_space, [[maybe_unused]] Frame*& frame
#define REF_OPCODE_ARGS [[maybe_unused]] const Instruction*& instr, [[maybe_unused]] MemorySpace*& mem_space, [[maybe_unused]] Frame*& frame
#define CONST_OPCODE_ARGS [[maybe_unused]] const Instruction* instr, [[maybe_unused]] const MemorySpace* mem_space, [[maybe_unused]] const Frame* frame
#define FRWARD_ARGS instr, mem_space, frame

#define EXEC_NEXT(step) instr += step; MUST_TAIL return EXEC(instr, mem_space, frame)
#define EXEC_START(instr, mem_space, frame) EXEC(program, mem_space, frame)


///////////////////// INSTRUCTION_DEFINITION ////////////////////////////

using Fun = void(OPCODE_ARGS);
using RawFun = void(REF_OPCODE_ARGS);
using bit64 = uint64_t;

constexpr size_t args_per_instr = 2;

struct Instruction {
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

using func_id = bit64;
using label_id = bit64;

// struct code_pos {
// 	uint64_t pos;
// 	func_id func;
// };

struct {
	std::unordered_map<std::string, func_id> functions = {{"main", 0}, {"test", 1}}; // temporary change for compilation purpose
	// std::unordered_map<label_id, code_pos> labels;
	// uint64_t curr_pos;
} ctx;

///////////////////// PARSING INSTRUCTIONS ////////////////////////////

enum class arg_t {
	DECIMAL_NUM,
	REGISTER_ID,
	FUNC_NAME,
};

std::string regex_arg_t(const arg_t& arg) {
	switch (arg) {
		case arg_t::DECIMAL_NUM: return "(\\d+)";
		case arg_t::REGISTER_ID: return "\\$(\\d+)";
		case arg_t::FUNC_NAME: return "([A-Za-z0-9_]+)";
		default:
			assert(false);			
	}
};

bit64 parse_arg_t(arg_t type, std::string input) {
	switch (type) {
		case arg_t::DECIMAL_NUM: return std::stoull(input);
		case arg_t::REGISTER_ID: return std::stoull(input);
		case arg_t::FUNC_NAME: return ctx.functions.emplace(input, ctx.functions.size()).first->second;
		default:
			assert(false);
	}
};

std::string to_string(arg_t c) {
    switch (c) {
        case arg_t::DECIMAL_NUM:	return "Decimal";
        case arg_t::REGISTER_ID:	return "Register";
        case arg_t::FUNC_NAME:		return "Function";
        default:					return "WTF?";
    }
}

template <std::same_as<arg_t>... ArgsT>
std::pair<std::regex, std::function<Instruction(std::smatch)>> make_opcode(std::string op_name, RawFun* code, ArgsT... args_t) {
	std::string ans = "\\s*" + op_name;

	(ans += ... += ("\\s+" + regex_arg_t(args_t) + ","));
	if (sizeof...(args_t) != 0) {
		ans.pop_back(); 		// removing last "," if present
	}
	ans += "\\s*(?:#.*)?";	// komentarze są dozwolone

	auto converter = [=](const std::smatch &match) -> Instruction {
		Instruction instr;
		instr.action = code;
		for (size_t i = 0; i < args_per_instr; i++) {
			instr.arg[i] = 0;
		}

		std::cout << sizeof...(args_t) << "\n";
		((std::cout << to_string(args_t) << " "), ...);
		std::cout << "\n";

		int i = 0;
		((instr.arg[i] = parse_arg_t(args_t, match[i + 1]), ++i, true) && ...);

		return instr;
	};

	std::cout << op_name << " -> \"" << ans << "\"\n";

	return {std::regex(ans), converter};
}

std::vector<std::pair<std::regex, std::function<Instruction(std::smatch)>>> opcodes = {
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

Instruction make_instr(const string& line) {
	std::smatch matches;
	for (auto &[pattern, func] : opcodes) {
		std::cout << "\"" << line << "\"\n";
		if (std::regex_match(line, matches, pattern)) {
			std::cout << "matched!\n";
			return func(matches);
		}
	}

	assert(false);
}

static const unordered_map<func_id, Code> func_id_to_code = {
    {0,
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
     }},
    {1,
     {
         make_instr("ir $2"),
         make_instr("init"),
         make_instr("rts $2"),
		 make_instr("ir $2"),
		 make_instr("init"),
         make_instr("rts $2"),
		 make_instr("ir $2"),
		 make_instr("init"),
         make_instr("rts $2"),
         make_instr("str $3"),
         make_instr("deinit"),
         make_instr("str $4"),
         make_instr("deinit"),
         make_instr("str $5"),
         make_instr("deinit"),
         make_instr("or $3"),
         make_instr("or $4"),
         make_instr("or $5"),
		 make_instr("ir $2"),
         make_instr("rtrv $2"),
         make_instr("ret"),
     }}
};
static const unordered_map<uint64_t, uint64_t> func_id_to_exp_arg = {
    {0, 0},
    {1, 0},
};

void start_execution(MemorySpace& mem_space) {
	auto instr = func_id_to_code.at(0).data();
	auto frame = mem_space.STK.get();
    auto memory = &mem_space;
	
	frame->stack_head = mem_space.MEM.get();
	frame->stack_start = mem_space.MEM.get();

	
	instr->action(instr, memory, frame);
}

//////////////////////////// OPCODE ACTION IMPLEMENTATION ////////////////////////////

static inline __attribute__((always_inline))  uint64_t* stk_top(CONST_OPCODE_ARGS) {
	assert(frame->stack_start < frame->stack_head);
	return (frame->stack_head - 1);
}

inline __attribute__((always_inline)) void rawFun::init(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM.get() + mem_space->MEM_SIZE;
	assert(frame->stack_head + 1 < end);

	frame->stack_head++;
}

inline __attribute__((always_inline)) void rawFun::deinit(REF_OPCODE_ARGS) {
	frame->stack_head = stk_top(FRWARD_ARGS);
}

inline __attribute__((always_inline)) void rawFun::reg_to_stk(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REG_SIZE);
    
	*stk_top(FRWARD_ARGS) = mem_space->REG[arg0];
}

inline __attribute__((always_inline)) void rawFun::stk_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REG_SIZE);

	mem_space->REG[arg0] = *stk_top(FRWARD_ARGS);
}

inline __attribute__((always_inline)) void rawFun::reg_to_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];
	assert(arg0 < mem_space->REG_SIZE);
	assert(arg1 < mem_space->REG_SIZE);

	mem_space->REG[arg1] = mem_space->REG[arg0];
}

inline __attribute__((always_inline)) void rawFun::reg_to_retval(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REG_SIZE);
    
	*frame->stack_start = mem_space->REG[arg0];
}

inline __attribute__((always_inline)) void rawFun::input_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REG_SIZE);

	size_t val;
	cin >> val;

	mem_space->REG[arg0] = val;
}

inline __attribute__((always_inline)) void rawFun::output_reg(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	assert(arg0 < mem_space->REG_SIZE);

	cout << mem_space->REG[arg0] << endl;
}

inline __attribute__((always_inline)) void rawFun::call(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = func_id_to_code.find(arg0);
	assert(it != func_id_to_code.end());

	frame->instr = instr + 1;

	auto prev_frame = frame;
	auto end = mem_space->STK.get() + mem_space->STK_SIZE;
	assert(frame + 1 < end);
	frame++;

	frame->stack_head = prev_frame->stack_head;

	auto shared_memory = 1 + func_id_to_exp_arg.at(arg0);        
	assert(frame->stack_start + shared_memory <= frame->stack_head);
	frame->stack_start = frame->stack_head - shared_memory;

	auto& [_, func_code] = *it;
	instr = func_code.data();
}

inline __attribute__((always_inline)) void rawFun::ret(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM.get() + mem_space->MEM_SIZE;
	assert(frame->stack_start + 1 < end);

	frame->stack_head = frame->stack_start + 1;
	frame--;
	instr = frame->instr;
}

inline __attribute__((always_inline)) void rawFun::exit(REF_OPCODE_ARGS) {
  	assert(frame->stack_head == mem_space->MEM.get() + 1);
	assert(frame == mem_space->STK.get());
	
	cout << format("program exited with {}\n", *mem_space->MEM.get());
	
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

MemorySpace global_space = MemorySpace(10, 10, 20);

int main() {
	start_execution(global_space);

	return 0;
}