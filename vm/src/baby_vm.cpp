#include "inline.h"
#include "musttail.h"
#include <bits/stdc++.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

///////////////////// DECLARATIONS ////////////////////////////

struct instrutction_t;
struct frame_t;
struct memory_space;
struct thread_t;
struct thread_builder_t;
struct function_t;
struct flag_data;
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
#define _N_PARSE_JUMPS _N_PARSE::jumps
#define _N_PARSE_UTILS _N_PARSE::utils

namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using parse_instr_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE

using bit64 = uint64_t;
using code_block = std::vector<instrutction_t>;

using func_id_t = uint64_t;
using labl_id_t = uint64_t;
using reg_id_t = uint64_t;

using reg_id_map = std::unordered_map<std::string, reg_id_t>;

using labl_id_map = std::unordered_map<std::string, labl_id_t>;
using labl_map = std::unordered_map<func_id_t, size_t>;

using func_id_map = std::unordered_map<std::string, func_id_t>;
using func_map = std::unordered_map<func_id_t, function_t>;

///////////////////// IMPL ////////////////////////////

struct flag_data {
	bool were_equal = false;
	bool first_was_bigger = false;
};

struct frame_t {
	const instrutction_t *instr;
	bit64 *stack_start;
	bit64 *stack_head;
	flag_data flags;
	func_id_t cur_func_id;
};

struct function_t {
	code_block body;
	uint64_t no_of_args;
	std::vector<bit64> regs;
};

struct heap_t {
	std::unique_ptr<bit64[]> content;
	std::set<std::pair<bit64, bit64>> allocated;
	const size_t size;

	heap_t(size_t alloc = 2048) : content(std::make_unique<bit64[]>(alloc)), allocated{}, size{alloc} {
		assert(content);
	}

	bit64 allocate(bit64 chunk_size) {
		auto it = allocated.begin();
		int start = 0;

		while (it != allocated.end() &&	 start + chunk_size > it->first) {
			start = it->first + it->second;
			it++;
		}

		assert(start + chunk_size <= size);
		allocated.insert({start, chunk_size});

		return start;
	}

	void deallocate(bit64 ptr) {
		for (auto it = allocated.begin(); it != allocated.end(); it++) {
			if (it->first == ptr) {
				allocated.erase(it);
				return;
			}
		}

		assert(false);
	}
};

struct memory_space {
	const size_t MEM_STACK_SIZE;
	const size_t CALL_STACK_SIZE;

	std::unique_ptr<bit64[]> MEM_STACK;
	std::unique_ptr<frame_t[]> CALL_STACK;
	heap_t heap;

	memory_space(size_t MEM_STACK_SIZE = 10, size_t CALL_STACK_SIZE = 1000) :
		MEM_STACK_SIZE(MEM_STACK_SIZE),
		CALL_STACK_SIZE(CALL_STACK_SIZE),
		MEM_STACK(std::make_unique<bit64[]>(MEM_STACK_SIZE)),
		CALL_STACK(std::make_unique<frame_t[]>(CALL_STACK_SIZE)),
		heap{} {
		assert(MEM_STACK && CALL_STACK);
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
		func_id_t main_id = func_id.at("main");
		const instrutction_t *instr = functions.at(main_id).body.data();
		auto frame = memory.CALL_STACK.get();
		auto mem = &memory;

		frame->stack_head = memory.MEM_STACK.get();
		frame->stack_start = memory.MEM_STACK.get();
		frame->cur_func_id = main_id;

		EXEC_START(instr, mem, frame, this);
	}
};

struct thread_builder_t {
public:
	func_map functions_impl;
	func_id_map func_decl;

	labl_id_map labl_decl;
	labl_map label_pos;

	reg_id_map reg_decl;

	std::vector<size_t> jump_ops{};

	code_block built_func_body{};

	void add_instr(instrutction_t &instr) { built_func_body.push_back(instr); }

	void add_jump(instrutction_t &instr) {
		jump_ops.push_back(built_func_body.size());
		add_instr(instr);
	}

	void define_labl(const std::string &name) {
		label_pos[refer_label(name)] = built_func_body.size();
	}

	void define_reg(const std::string &name) {
		assert(reg_decl.emplace(name, reg_decl.size()).second);
	}

	void commit_func(const std::string &name, uint64_t no_of_args) {
		for (const auto &[name, id] : labl_decl) {
			assert(label_pos.find(id) != label_pos.end());
		}
		while (jump_ops.size()) {
			int curr_pos = jump_ops.back();
			jump_ops.pop_back();
			instrutction_t &jmp_instr = built_func_body[curr_pos];
			labl_id_t label_id = jmp_instr.arg[0];
			jmp_instr.arg[0] = label_pos[label_id] - curr_pos;
		}

		functions_impl.emplace(refer_func(name), function_t{
													 .body = built_func_body,
													 .no_of_args = no_of_args,
													 .regs = std::vector<bit64>(reg_decl.size()),
												 });

		built_func_body.clear();
		reg_decl.clear();
		label_pos.clear();
		labl_decl.clear();
	};

	thread_t make_thread() {
		assert(built_func_body.empty() && reg_decl.empty() && label_pos.empty() &&
			   labl_decl.empty() && jump_ops.empty());

		return {
			.functions = functions_impl,
			.func_id = func_decl,
			.memory = memory_space(),
		};
	}

	func_id_t refer_func(const std::string &input) {
		return func_decl.emplace(input, func_decl.size()).first->second;
	}

	labl_id_t refer_label(const std::string &input) {
		return labl_decl.emplace(input, labl_decl.size()).first->second;
	}

	reg_id_t refer_reg(const std::string &input) {
		auto it = reg_decl.find(input);
		assert(it != reg_decl.end());
		return it->second;
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
#define JUMP_EQ jump_if_eq
#define CMP_REG cmp_reg
#define DEMAND_REG demand_reg
#define INIT_IMM init_imm
#define ADD_IN_PLACE add_in_place
#define MUL_IN_PLACE mul_in_place
#define SUB_IN_PLACE sub_in_place
#define MOD_IN_PLACE mod_in_place
#define DIV_IN_PLACE div_in_place
#define ALLOC_HEAP alloc_heap
#define DEALLOC_HEAP dealloc_heap


#define _N_ARGS args
#define _N_ARGS_UTILS _N_ARGS::utils

#define STRINGIFY(x) #x
#define nameof(x) STRINGIFY(x)

#define REQUIRED_FOR_EXEC(macro)                                                                   \
	namespace _N_EXEC_RAW {                                                                        \
	INLINE RawFun macro;                                                                           \
	}                                                                                              \
	namespace _N_EXEC_FULL {                                                                       \
	Fun macro;                                                                                     \
	}                                                                                              \
	namespace _N_EXEC_UTILS::next_instr_offset {                                                   \
	constexpr size_t macro();                                                                      \
	}

#define REQUIRE_ARGS(macro)                                                                        \
	namespace _N_ARGS {                                                                            \
	const std::vector<operand_t> &macro();                                                         \
	}

#define REQUIRED_FOR_SIMPLE(macro)                                                                 \
	REQUIRED_FOR_EXEC(macro)                                                                       \
	REQUIRE_ARGS(macro)                                                                            \
	namespace _N_PARSE_SIMPLE {                                                                    \
	parse_instr_t macro;                                                                           \
	}

#define REQUIRED_FOR_JUMPS(macro)                                                                  \
	REQUIRED_FOR_EXEC(macro)                                                                       \
	REQUIRE_ARGS(macro)                                                                            \
	namespace _N_PARSE_JUMPS {                                                                     \
	parse_instr_t macro;                                                                           \
	}

REQUIRE_ARGS(LABEL)
REQUIRE_ARGS(DEMAND_REG)
REQUIRE_ARGS(INIT_IMM)
namespace _N_PARSE_OTHER {
parse_instr_t LABEL;
parse_instr_t DEMAND_REG;

} // namespace _N_PARSE_OTHER

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
REQUIRED_FOR_SIMPLE(CMP_REG)
REQUIRED_FOR_SIMPLE(INIT_IMM)
REQUIRED_FOR_SIMPLE(ALLOC_HEAP)
REQUIRED_FOR_SIMPLE(DEALLOC_HEAP)

REQUIRED_FOR_SIMPLE(ADD_IN_PLACE)
REQUIRED_FOR_SIMPLE(MUL_IN_PLACE)
REQUIRED_FOR_SIMPLE(DIV_IN_PLACE)
REQUIRED_FOR_SIMPLE(SUB_IN_PLACE)
REQUIRED_FOR_SIMPLE(MOD_IN_PLACE)

REQUIRED_FOR_JUMPS(JUMP)
REQUIRED_FOR_JUMPS(JUMP_EQ)
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
		return "\\$([A-Za-z0-9_]+)";
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
		return builder.refer_reg(input);
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
ENLIST_OPERANDS(CMP_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(INPUT_TO_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(OUTPUT_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(FUNC_RET)
ENLIST_OPERANDS(EXIT_PROG)
ENLIST_OPERANDS(CALL, operand_t::FUNC_NAME)

ENLIST_OPERANDS(JUMP, operand_t::LABEL_ID)
ENLIST_OPERANDS(JUMP_EQ, operand_t::LABEL_ID)

ENLIST_OPERANDS(LABEL, operand_t::LABEL_ID)
ENLIST_OPERANDS(DEMAND_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(INIT_IMM, operand_t::REGISTER_ID, operand_t::DECIMAL_NUM)

ENLIST_OPERANDS(ALLOC_HEAP, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DEALLOC_HEAP, operand_t::REGISTER_ID)

ENLIST_OPERANDS(ADD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MUL_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DIV_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(SUB_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MOD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)

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
	{nameof(CMP_REG), CMP_REG()},
	{nameof(EXIT_PROG), EXIT_PROG()},
	{nameof(CALL), CALL()},
	{nameof(LABEL), LABEL()},
	{nameof(JUMP), JUMP()},
	{nameof(JUMP_EQ), JUMP_EQ()},
	{nameof(DEMAND_REG), DEMAND_REG()},
	{nameof(ADD_IN_PLACE), ADD_IN_PLACE()},
	{nameof(MUL_IN_PLACE), MUL_IN_PLACE()},
	{nameof(SUB_IN_PLACE), SUB_IN_PLACE()},
	{nameof(DIV_IN_PLACE), DIV_IN_PLACE()},
	{nameof(MOD_IN_PLACE), MOD_IN_PLACE()},
	{nameof(INIT_IMM), INIT_IMM()},	
	{nameof(ALLOC_HEAP), ALLOC_HEAP()},	
	{nameof(DEALLOC_HEAP), DEALLOC_HEAP()},	
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
PARSE_SIMPLE_IMPL(CMP_REG)
PARSE_SIMPLE_IMPL(ADD_IN_PLACE)
PARSE_SIMPLE_IMPL(MUL_IN_PLACE)
PARSE_SIMPLE_IMPL(SUB_IN_PLACE)
PARSE_SIMPLE_IMPL(DIV_IN_PLACE)
PARSE_SIMPLE_IMPL(MOD_IN_PLACE)
PARSE_SIMPLE_IMPL(INIT_IMM)
PARSE_SIMPLE_IMPL(ALLOC_HEAP)
PARSE_SIMPLE_IMPL(DEALLOC_HEAP)

#define PARSE_JUMPS_IMPL(macro)                                                                    \
	void _N_PARSE_JUMPS::macro(PARSE_OPCODE_ARGS) {                                                \
		instrutction_t instr;                                                                      \
		instr.action = &::_N_EXEC_FULL::macro;                                                     \
		for (size_t i = 0; i < args_per_instr; i++) {                                              \
			instr.arg[i] = 0;                                                                      \
		}                                                                                          \
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
		builder.add_jump(instr);                                                                   \
	}

PARSE_JUMPS_IMPL(JUMP)
PARSE_JUMPS_IMPL(JUMP_EQ)

void _N_PARSE_OTHER::LABEL(PARSE_OPCODE_ARGS) { builder.define_labl(matches[1]); }
void _N_PARSE_OTHER::DEMAND_REG(PARSE_OPCODE_ARGS) { builder.define_reg(matches[1]); }

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
			{nameof(CMP_REG), {make_opcode_pattern(nameof(CMP_REG)), &CMP_REG}},
			{nameof(ADD_IN_PLACE), {make_opcode_pattern(nameof(ADD_IN_PLACE)), &ADD_IN_PLACE}},
			{nameof(MUL_IN_PLACE), {make_opcode_pattern(nameof(MUL_IN_PLACE)), &MUL_IN_PLACE}},
			{nameof(MOD_IN_PLACE), {make_opcode_pattern(nameof(MOD_IN_PLACE)), &MOD_IN_PLACE}},
			{nameof(DIV_IN_PLACE), {make_opcode_pattern(nameof(DIV_IN_PLACE)), &DIV_IN_PLACE}},
			{nameof(SUB_IN_PLACE), {make_opcode_pattern(nameof(SUB_IN_PLACE)), &SUB_IN_PLACE}},
			{nameof(INIT_IMM), {make_opcode_pattern(nameof(INIT_IMM)), &INIT_IMM}},
			{nameof(ALLOC_HEAP), {make_opcode_pattern(nameof(ALLOC_HEAP)), &ALLOC_HEAP}},
			{nameof(DEALLOC_HEAP), {make_opcode_pattern(nameof(DEALLOC_HEAP)), &DEALLOC_HEAP}},

			{nameof(JUMP), {make_opcode_pattern(nameof(JUMP)), &::_N_PARSE_JUMPS::JUMP}},
			{nameof(JUMP_EQ), {make_opcode_pattern(nameof(JUMP_EQ)), &::_N_PARSE_JUMPS::JUMP_EQ}},
			{nameof(LABEL), {make_opcode_pattern(nameof(LABEL)), &::_N_PARSE_OTHER::LABEL}},
			{nameof(DEMAND_REG),
			 {make_opcode_pattern(nameof(DEMAND_REG)), &::_N_PARSE_OTHER::DEMAND_REG}},
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

		std::stringstream code(matches[3]);
		static std::string line;
		while (getline(code, line)) {
			parse_line(line, res);
		}
		res.commit_func(matches[1], stoull(matches[2]));
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

INLINE static bit64 *last_on_stck(CONST_OPCODE_ARGS) {
	assert(frame->stack_start < frame->stack_head);
	return (frame->stack_head - 1);
}

INLINE static bit64 &get_reg(bit64 arg0, REF_OPCODE_ARGS) {
	auto cur_func = frame->cur_func_id;
	auto &all_func = exec_program->functions;

	auto it = all_func.find(cur_func);
	assert(it != all_func.end());

	auto &function = it->second;
	assert(arg0 < function.regs.size());

	return function.regs[arg0];
}

void _N_EXEC_RAW::STCK_INIT(REF_OPCODE_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	assert(frame->stack_head + 1 < end);

	frame->stack_head++;
}

void _N_EXEC_RAW::STCK_DEINIT(REF_OPCODE_ARGS) { frame->stack_head = last_on_stck(FRWARD_ARGS); }

void _N_EXEC_RAW::REG_TO_STCK(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	*last_on_stck(FRWARD_ARGS) = get_reg(arg0, FRWARD_ARGS);
}

void _N_EXEC_RAW::STCK_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	get_reg(arg0, FRWARD_ARGS) = *last_on_stck(FRWARD_ARGS);
}

void _N_EXEC_RAW::REG_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg1, FRWARD_ARGS) = get_reg(arg0, FRWARD_ARGS);
}

void _N_EXEC_RAW::REG_TO_RVAL(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	*frame->stack_start = get_reg(arg0, FRWARD_ARGS);
}

void _N_EXEC_RAW::INPUT_TO_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	size_t val;
	std::cin >> val;

	get_reg(arg0, FRWARD_ARGS) = val;
}

void _N_EXEC_RAW::OUTPUT_REG(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	std::cout << get_reg(arg0, FRWARD_ARGS) << std::endl;
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

void _N_EXEC_RAW::CMP_REG(REF_OPCODE_ARGS) {
	auto &arg0 = instr->arg[0];
	auto &arg1 = instr->arg[1];

	const auto &reg0 = get_reg(arg0, FRWARD_ARGS);
	const auto &reg1 = get_reg(arg1, FRWARD_ARGS);

	frame->flags.were_equal = (reg0 == reg1);
	frame->flags.first_was_bigger = (reg0 > reg1);
}

void _N_EXEC_RAW::JUMP_EQ(REF_OPCODE_ARGS) {
	if (frame->flags.were_equal) {
		instr += instr->arg[0];
	} else {
		instr++;
	}
}

void _N_EXEC_RAW::INIT_IMM(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg1, FRWARD_ARGS) = arg1;
}

void _N_EXEC_RAW::ALLOC_HEAP(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg0, FRWARD_ARGS) = mem_space->heap.allocate(get_reg(arg1, FRWARD_ARGS));
}

void _N_EXEC_RAW::DEALLOC_HEAP(REF_OPCODE_ARGS) {
	auto arg0 = instr->arg[0];

	mem_space->heap.deallocate(get_reg(arg0, FRWARD_ARGS));
}

#define EXEC_ARITH_IN_PLACE_IMPL(macro, oper)                                                      \
	void _N_EXEC_RAW::macro(REF_OPCODE_ARGS) {                                                     \
		auto arg0 = instr->arg[0];                                                                 \
		auto arg1 = instr->arg[1];                                                                 \
		get_reg(arg0, FRWARD_ARGS) oper get_reg(arg1, FRWARD_ARGS);                                \
	}

EXEC_ARITH_IN_PLACE_IMPL(ADD_IN_PLACE, +=)
EXEC_ARITH_IN_PLACE_IMPL(MUL_IN_PLACE, *=)
EXEC_ARITH_IN_PLACE_IMPL(SUB_IN_PLACE, -=)
EXEC_ARITH_IN_PLACE_IMPL(DIV_IN_PLACE, /=)
EXEC_ARITH_IN_PLACE_IMPL(MOD_IN_PLACE, %=)

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
INSTR_OFFSET_IMPL(CMP_REG, 1)
INSTR_OFFSET_IMPL(ADD_IN_PLACE, 1)
INSTR_OFFSET_IMPL(MUL_IN_PLACE, 1)
INSTR_OFFSET_IMPL(MOD_IN_PLACE, 1)
INSTR_OFFSET_IMPL(SUB_IN_PLACE, 1)
INSTR_OFFSET_IMPL(DIV_IN_PLACE, 1)
INSTR_OFFSET_IMPL(INIT_IMM, 1)
INSTR_OFFSET_IMPL(ALLOC_HEAP, 1)
INSTR_OFFSET_IMPL(DEALLOC_HEAP, 1)


INSTR_OFFSET_IMPL(FUNC_RET, 0)
INSTR_OFFSET_IMPL(CALL, 0)
INSTR_OFFSET_IMPL(JUMP, 0)
INSTR_OFFSET_IMPL(JUMP_EQ, 0)

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
EXEC_FULL_IMPL(JUMP_EQ)
EXEC_FULL_IMPL(CMP_REG)
EXEC_FULL_IMPL(ADD_IN_PLACE)
EXEC_FULL_IMPL(MUL_IN_PLACE)
EXEC_FULL_IMPL(SUB_IN_PLACE)
EXEC_FULL_IMPL(MOD_IN_PLACE)
EXEC_FULL_IMPL(DIV_IN_PLACE)
EXEC_FULL_IMPL(INIT_IMM)
EXEC_FULL_IMPL(ALLOC_HEAP)
EXEC_FULL_IMPL(DEALLOC_HEAP)

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
