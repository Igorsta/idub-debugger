#include "defer.h"
#include "inline.h"
#include "musttail.h"
#include <bits/stdc++.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <sys/types.h>
#include <unordered_map>

///////////////////// DECLARATIONS ////////////////////////////

enum class exec_mode {
	NORMAL,
	DEBUG,
};

struct instrutction_t;
struct frame_t;
struct memory_space;
struct thread_dbg_data_t;

template <typename T1, typename T2> struct biject_map_t;

template <exec_mode mode = exec_mode::NORMAL> struct thread_t;

struct func_builder_t;
struct code_pos_t;
struct thread_builder_t;
struct function_t;
struct flag_data;
enum class operand_t;

///////////////////// USINGS ////////////////////////////

using bit64 = uint64_t;
using code_block = std::vector<instrutction_t>;

using func_id_t = uint64_t;
using labl_id_t = uint64_t;
using reg_id_t = uint64_t;
using breakpoints_id = uint64_t;

using reg_id_map = biject_map_t<std::string, reg_id_t>;

using labl_id_map = biject_map_t<std::string, labl_id_t>;
using labl_map = std::unordered_map<labl_id_t, size_t>;

using func_id_map = biject_map_t<std::string, func_id_t>;
using func_map = std::unordered_map<func_id_t, function_t>;

///////////////////// EXEC MACRO ////////////////////////////

#define CORE_ASSERT(cond, ...)                                                                     \
	if (!(cond)) {                                                                                 \
		std::println(__VA_ARGS__);                                                                 \
		assert(false);                                                                             \
	}

#define CONST_RAW_EXEC_ARGS                                                                        \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const func_map *avail_funcs

#define _N_EXEC		  exec
#define _N_EXEC_RAW	  _N_EXEC::raw
#define _N_EXEC_UTILS _N_EXEC::utils

namespace _N_EXEC_RAW {
#define RAW_EXEC_ARGS                                                                              \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *mem_space,       \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] func_map *avail_funcs
#define FWD_RAW_ARGS instr, mem_space, frame, avail_funcs
using ret_t = std::ptrdiff_t;
using func_t = ret_t(RAW_EXEC_ARGS);

}; // namespace _N_EXEC_RAW

#define _N_PARSE		parse
#define _N_PARSE_OTHER	_N_PARSE::other
#define _N_PARSE_SIMPLE _N_PARSE::simple
#define _N_PARSE_JUMPS	_N_PARSE::jumps
#define _N_PARSE_UTILS	_N_PARSE::utils

namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using parse_instr_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE

///////////////////// OPCODE_MACROS ////////////////////////////
#define STCK_INIT	 init
#define STCK_DEINIT	 deinit
#define REG_TO_STCK	 reg_to_stack
#define STCK_TO_REG	 stack_to_reg
#define REG_TO_RVAL	 reg_to_retval
#define REG_TO_REG	 reg_to_reg
#define INPUT_TO_REG input_reg
#define OUTPUT_REG	 output_reg
#define FUNC_RET	 func_return
#define EXIT_PROG	 exit_prog
#define CALL		 call_func
#define LABEL		 label
#define JUMP		 jump_to
#define JUMP_EQ		 jump_if_eq
#define CMP_REG		 cmp_reg
#define DEMAND_REG	 demand_reg
#define INIT_IMM	 init_imm
#define ADD_IN_PLACE add_in_place
#define MUL_IN_PLACE mul_in_place
#define SUB_IN_PLACE sub_in_place
#define MOD_IN_PLACE mod_in_place
#define DIV_IN_PLACE div_in_place
#define ALLOC_HEAP	 alloc_heap
#define DEALLOC_HEAP dealloc_heap
#define READ_HEAP	 read_heap
#define WRITE_HEAP	 write_heap

#define _N_ARGS		  args
#define _N_ARGS_UTILS _N_ARGS::utils

#define STRINGIFY(x) #x
#define nameof(x)	 STRINGIFY(x)

#define REQUIRED_FOR_EXEC(macro)                                                                   \
	namespace _N_EXEC_RAW {                                                                        \
	INLINE func_t macro;                                                                           \
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
REQUIRED_FOR_SIMPLE(READ_HEAP)
REQUIRED_FOR_SIMPLE(WRITE_HEAP)

REQUIRED_FOR_SIMPLE(ADD_IN_PLACE)
REQUIRED_FOR_SIMPLE(MUL_IN_PLACE)
REQUIRED_FOR_SIMPLE(DIV_IN_PLACE)
REQUIRED_FOR_SIMPLE(SUB_IN_PLACE)
REQUIRED_FOR_SIMPLE(MOD_IN_PLACE)

REQUIRED_FOR_JUMPS(JUMP)
REQUIRED_FOR_JUMPS(JUMP_EQ)

///////////////////// IMPL ////////////////////////////

struct instrutction_t {
	constexpr static size_t args_per_instr = 2;
	_N_EXEC_RAW::func_t *action;
	bit64 arg[args_per_instr];

	instrutction_t(_N_EXEC_RAW::func_t *action) : action{action} {
		for (int i = 0; i < args_per_instr; i++) {
			arg[i] = 0;
		}
	}
};

struct code_pos_t {
	func_id_t func_id;
	size_t pos;

	bool operator==(const code_pos_t &oth) const {
		return oth.func_id == func_id && pos == oth.pos;
	}
};

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
	uint64_t res_size;
	mutable std::vector<bit64> regs;
};

template <typename T1, typename T2> struct biject_map_t {
	std::unordered_map<T1, T2> map_1_to_2;
	std::unordered_map<T2, T1> map_2_to_1;

	std::optional<T2> by_first(T1 key1) {
		auto it = map_1_to_2.find(key1);
		return (it == map_1_to_2.end()) ? std::optional<T2>{} : it->second;
	}

	std::optional<T1> by_second(T2 key2) {
		auto it = map_2_to_1.find(key2);
		return (it == map_2_to_1.end()) ? std::optional<T1>{} : it->second;
	}

	size_t size() {
		CORE_ASSERT(map_1_to_2.size() == map_2_to_1.size(),
					"bijection requires that both sets are equally big");
		return map_1_to_2.size();
	}

	std::pair<bool, T2> emplace_first(T1 key1, T2 val2) {
		CORE_ASSERT(map_2_to_1.find(val2) == map_2_to_1.end(),
					"Trying to bind {} but it is already binded in right set", val2);

		auto it1 = map_1_to_2.find(key1);
		if (it1 != map_1_to_2.end()) {
			return std::make_tuple(false, it1->second);
		}

		map_1_to_2.emplace(key1, val2);
		map_2_to_1.emplace(val2, key1);
		return std::make_tuple(true, val2);
	}

	void clear() {
		map_1_to_2.clear();
		map_2_to_1.clear();
	}

	bool empty() { return (size() == 0); }
};

namespace std {
template <> struct hash<code_pos_t> {
	constexpr size_t operator()(const code_pos_t &code_pos) const {
		return std::hash<func_id_t>{}(code_pos.func_id) ^ code_pos.pos;
	}
};
} // namespace std

struct thread_dbg_data_t {

	struct func_dbg_data_t {
		reg_id_map registers;
		labl_id_map labels;
		labl_map label_positions;
	};

	std::unordered_map<func_id_t, func_dbg_data_t> func_data;
	biject_map_t<breakpoints_id, code_pos_t> breakpoints;
	func_id_map function_names;

	std::optional<breakpoints_id> hit_breakpoint(const code_pos_t &position) {
		return breakpoints.by_second(position);
	}

	void dbg_cntrl(code_pos_t position, thread_t<exec_mode::DEBUG>& thread) {
		bool run_cli = false;
		if (auto breakpoint = hit_breakpoint(position); breakpoint.has_value()) {
			std::cout << "Just hit a breakpoint!\n";
			run_cli = true;
		}

		if (run_cli) [[unlikely]] {
			cli();
		}
	}

	void cli() {
		;
	}
};

struct heap_t {
	std::unique_ptr<bit64[]> content;
	std::set<std::pair<bit64, bit64>> allocated;
	const size_t size;

	heap_t(size_t alloc = 2048) :
		content(std::make_unique<bit64[]>(alloc)),
		allocated{},
		size{alloc} {CORE_ASSERT(content, "Couldn't successfully allocate buffer for heap!")}

		bit64 allocate(bit64 chunk_size) {
		auto it = allocated.begin();
		int start = 0;

		while (it != allocated.end() && start + chunk_size > it->first) {
			start = it->first + it->second;
			it++;
		}

		CORE_ASSERT(start + chunk_size <= size,
					"Failed to allocate chunk: {0} + {1} is bigger than {2}", start, chunk_size,
					size);
		allocated.insert({start, chunk_size});

		return start;
	}

	void deallocate(bit64 ptr) {
		auto it = allocated.lower_bound({ptr, 0});

		CORE_ASSERT(it != allocated.end() && it->first == ptr,
					"address {} was not used for allocation", ptr);

		allocated.erase(it);
		return;
	}

	bit64 read(bit64 ptr) {
		auto it = allocated.lower_bound({ptr, 0});

		CORE_ASSERT(it != allocated.begin() && (--it)->first <= ptr && ptr < it->first + it->second,
					"address {} was not allocated", ptr);

		return content[ptr];
	}

	void write(bit64 ptr, bit64 val) {
		auto it = allocated.lower_bound({ptr, 0});

		CORE_ASSERT(it != allocated.begin() && (--it)->first <= ptr && ptr < it->first + it->second,
					"address {} was not allocated", ptr);

		content[ptr] = val;
		return;
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
		CORE_ASSERT(MEM_STACK && CALL_STACK, "Couldn't allocate memory and call stacks");
	}
};

template <exec_mode mode> struct thread_t {
	func_map functions;
	func_id_t main_id;
	memory_space memory;

	
	static thread_dbg_data_t &dummy_dbg_data() {
		static thread_dbg_data_t dummy;
		return dummy;
	}

	void start_execution(thread_dbg_data_t& dbg = dummy_dbg_data()) {
		const instrutction_t *instr = functions.at(main_id).body.data();
		auto frame = memory.CALL_STACK.get();
		auto mem = &memory;

		frame->stack_head = memory.MEM_STACK.get();
		frame->stack_start = memory.MEM_STACK.get();
		frame->cur_func_id = main_id;

		try {
			if constexpr (mode == exec_mode::NORMAL) {
				exec(instr, frame);
			} else {
				exec(instr, frame, dbg);
			}
		} catch (bit64 result) {
			// std::print("Now the VM knows that the program ended with {}\n", result);
		}
	}

	void exec(const instrutction_t *&instr, frame_t *&frame) requires (mode == exec_mode::NORMAL) {
		{
			instr += instr->action(instr, &memory, frame, &functions);
		}

		MUST_TAIL return exec(instr, frame);
	}

	void exec(const instrutction_t *&instr, frame_t *&frame, thread_dbg_data_t& dbg) requires (mode == exec_mode::DEBUG) {
		try {
			instr += instr->action(instr, &memory, frame, &functions);
		} catch (bit64 result) {
			return;
		}

		MUST_TAIL return exec(instr, frame, dbg);
	}

	code_pos_t get_position(const instrutction_t *&instr, frame_t *&frame) {
		auto func_id = frame->cur_func_id;
		return code_pos_t { .func_id = func_id, .pos = instr - functions[func_id].body.data()};
	}
};

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
		CORE_ASSERT(held.has_value(), "Using undeclared reguster {}", input);
		return held.value();
	}

	thread_dbg_data_t::func_dbg_data_t make_debug_data() {
		return {
			.registers = reg_decl,
			.labels = labl_decl,
			.label_positions = label_pos,
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
								   .regs = std::vector<bit64>(reg_decl.size()),
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

	template <exec_mode MODE> thread_t<MODE> make_thread() {
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

enum class operand_t {
	DECIMAL_NUM,
	REGISTER_ID,
	FUNC_NAME,
	LABEL_ID,
	REG_WITH_PTR,
};

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

constexpr std::string regex_func() {
	return "\\s*fun\\s+" + regex_arg_t(operand_t::FUNC_NAME) + "\\s*\\(\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*\\)" + "\\s*->\\s*" +
		   regex_arg_t(operand_t::DECIMAL_NUM) + "\\s*:\\s*" + "\\s*\\{([^}]*)\\}";
}

namespace _N_PARSE_UTILS {
constexpr bit64 parse_arg_t(thread_builder_t &builder, operand_t type, std::string input) {
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
ENLIST_OPERANDS(EXIT_PROG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(CALL, operand_t::FUNC_NAME)

ENLIST_OPERANDS(JUMP, operand_t::LABEL_ID)
ENLIST_OPERANDS(JUMP_EQ, operand_t::LABEL_ID)

ENLIST_OPERANDS(LABEL, operand_t::LABEL_ID)
ENLIST_OPERANDS(DEMAND_REG, operand_t::REGISTER_ID)
ENLIST_OPERANDS(INIT_IMM, operand_t::REGISTER_ID, operand_t::DECIMAL_NUM)

ENLIST_OPERANDS(ALLOC_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DEALLOC_HEAP, operand_t::REG_WITH_PTR)
ENLIST_OPERANDS(READ_HEAP, operand_t::REGISTER_ID, operand_t::REG_WITH_PTR)
ENLIST_OPERANDS(WRITE_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)

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
	{nameof(READ_HEAP), READ_HEAP()},
	{nameof(WRITE_HEAP), WRITE_HEAP()},
};

}; // namespace _N_ARGS_UTILS

//////////////////////////// PARSE SIMPLE OPCODE IMPL ////////////////////////////

#define PARSE_SIMPLE_IMPL(macro)                                                                   \
	void _N_PARSE_SIMPLE::macro(PARSE_OPCODE_ARGS) {                                               \
		instrutction_t instr{&::_N_EXEC_RAW::macro};                                               \
                                                                                                   \
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			instr.arg[i] = _N_PARSE_UTILS::parse_arg_t(builder, args[i], matches[i + 1]);          \
		}                                                                                          \
		builder.func_builder.add_instr(instr);                                                     \
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
PARSE_SIMPLE_IMPL(WRITE_HEAP)
PARSE_SIMPLE_IMPL(READ_HEAP)

#define PARSE_JUMPS_IMPL(macro)                                                                    \
	void _N_PARSE_JUMPS::macro(PARSE_OPCODE_ARGS) {                                                \
		instrutction_t instr{&::_N_EXEC_RAW::macro};                                               \
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

PARSE_JUMPS_IMPL(JUMP)
PARSE_JUMPS_IMPL(JUMP_EQ)

void _N_PARSE_OTHER::LABEL(PARSE_OPCODE_ARGS) { builder.func_builder.define_labl(matches[1]); }
void _N_PARSE_OTHER::DEMAND_REG(PARSE_OPCODE_ARGS) { builder.func_builder.define_reg(matches[1]); }

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
			{nameof(WRITE_HEAP), {make_opcode_pattern(nameof(WRITE_HEAP)), &WRITE_HEAP}},
			{nameof(READ_HEAP), {make_opcode_pattern(nameof(READ_HEAP)), &READ_HEAP}},

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

	CORE_ASSERT(false, "Line \"{}\" could not be parsed", line);
}

template <exec_mode MODE> thread_t<MODE> file_parse(std::string &content) {
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

	return res.make_thread<MODE>();
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

INLINE static bit64 *last_on_stck(CONST_RAW_EXEC_ARGS) {
	CORE_ASSERT(frame->stack_start < frame->stack_head,
				"Trying to access part of the stack what belongs to other function");
	return (frame->stack_head - 1);
}

INLINE static bit64 &get_reg(bit64 arg0, CONST_RAW_EXEC_ARGS) {
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

_N_EXEC_RAW::ret_t _N_EXEC_RAW::STCK_INIT(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	CORE_ASSERT(frame->stack_head + 1 < end, "Stack overflow");

	frame->stack_head++;

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::STCK_DEINIT(RAW_EXEC_ARGS) {
	frame->stack_head = last_on_stck(FWD_RAW_ARGS);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::REG_TO_STCK(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	*last_on_stck(FWD_RAW_ARGS) = get_reg(arg0, FWD_RAW_ARGS);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::STCK_TO_REG(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	get_reg(arg0, FWD_RAW_ARGS) = *last_on_stck(FWD_RAW_ARGS);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::REG_TO_REG(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg1, FWD_RAW_ARGS) = get_reg(arg0, FWD_RAW_ARGS);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::REG_TO_RVAL(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	*frame->stack_start = get_reg(arg0, FWD_RAW_ARGS);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::INPUT_TO_REG(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	size_t val;
	std::cin >> val;

	get_reg(arg0, FWD_RAW_ARGS) = val;

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::OUTPUT_REG(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	std::cout << get_reg(arg0, FWD_RAW_ARGS) << std::endl;

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::CALL(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = avail_funcs->find(arg0);
	CORE_ASSERT(it != avail_funcs->end(), "Call to undeclared function");

	frame->instr = instr + 1;

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

_N_EXEC_RAW::ret_t _N_EXEC_RAW::FUNC_RET(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	auto &func = (*avail_funcs)[frame->cur_func_id];

	auto expected_head = frame->stack_start + func.res_size;
	CORE_ASSERT(expected_head == frame->stack_head,
				"Function returned wrong number of elements on a stack");

	frame--;
	frame->stack_head = expected_head;
	instr = frame->instr;

	return 0;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::EXIT_PROG(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto ret_val = get_reg(arg0, FWD_RAW_ARGS);

	std::cout << std::format("program exited with {}\n", ret_val);

	throw ret_val;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::JUMP(RAW_EXEC_ARGS) { return instr->arg[0]; }

_N_EXEC_RAW::ret_t _N_EXEC_RAW::CMP_REG(RAW_EXEC_ARGS) {
	auto &arg0 = instr->arg[0];
	auto &arg1 = instr->arg[1];

	const auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	const auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	frame->flags.were_equal = (reg0 == reg1);
	frame->flags.first_was_bigger = (reg0 > reg1);

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::JUMP_EQ(RAW_EXEC_ARGS) {
	return (frame->flags.were_equal) ? instr->arg[0] : 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::INIT_IMM(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg1, FWD_RAW_ARGS) = arg1;

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::ALLOC_HEAP(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg0, FWD_RAW_ARGS) = mem_space->heap.allocate(get_reg(arg1, FWD_RAW_ARGS));

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::DEALLOC_HEAP(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];

	mem_space->heap.deallocate(get_reg(arg0, FWD_RAW_ARGS));

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::WRITE_HEAP(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	mem_space->heap.write(get_reg(arg0, FWD_RAW_ARGS), get_reg(arg1, FWD_RAW_ARGS));

	return 1;
}

_N_EXEC_RAW::ret_t _N_EXEC_RAW::READ_HEAP(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	get_reg(arg0, FWD_RAW_ARGS) = mem_space->heap.read(get_reg(arg1, FWD_RAW_ARGS));

	return 1;
}

#define EXEC_ARITH_IN_PLACE_IMPL(macro, oper)                                                      \
	_N_EXEC_RAW::ret_t _N_EXEC_RAW::macro(RAW_EXEC_ARGS) {                                         \
		auto arg0 = instr->arg[0];                                                                 \
		auto arg1 = instr->arg[1];                                                                 \
		get_reg(arg0, FWD_RAW_ARGS) oper get_reg(arg1, FWD_RAW_ARGS);                              \
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

	if (mode == "run") {
		auto program = _N_PARSE_UTILS::file_parse<exec_mode::NORMAL>(file_content);
		program.start_execution();
	} else {
		auto program = _N_PARSE_UTILS::file_parse<exec_mode::DEBUG>(file_content);
		program.start_execution();
	}

	return 0;
}
