#include "defer.h"
#include "debug.h"
#include "inline.h"
#include "musttail.h"
#include "opcode_def.hpp"
#include <bits/stdc++.h>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <print>
#include <sys/types.h>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "biject_map.hpp"

///////////////////// DECLARATIONS ////////////////////////////

enum class exec_mode {
	NORMAL,
	DEBUG,
};

struct instrutction_t;
struct frame_t;
struct memory_space;
struct thread_dbg_data_t;
struct dbg_require_stop;

struct thread_t;

struct code_pos_t;
struct func_builder_t;
struct thread_builder_t;
struct function_t;
struct flag_data;
enum class operand_t;
struct io_handler_t;

///////////////////// USINGS ////////////////////////////

using unit = uint64_t;
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

#define CONST_RAW_EXEC_ARGS                                                                        \
	[[maybe_unused]] const instrutction_t *instr, [[maybe_unused]] const memory_space *mem_space,  \
		[[maybe_unused]] const frame_t *frame, [[maybe_unused]] const func_map *avail_funcs

#define _N_EXEC		  exec
#define _N_EXEC_UTILS _N_EXEC::utils

namespace _N_EXEC {
#define RAW_EXEC_ARGS                                                                              \
	[[maybe_unused]] const instrutction_t *&instr, [[maybe_unused]] memory_space *mem_space,       \
		[[maybe_unused]] frame_t *&frame, [[maybe_unused]] func_map *avail_funcs
#define FWD_RAW_ARGS instr, mem_space, frame, avail_funcs
using ret_t = std::ptrdiff_t;
using func_t = ret_t(RAW_EXEC_ARGS);

}; // namespace _N_EXEC

#define _N_PARSE	   parse
#define _N_PARSE_UTILS _N_PARSE::utils

namespace _N_PARSE {
#define PARSE_OPCODE_ARGS                                                                          \
	[[maybe_unused]] thread_builder_t &builder, [[maybe_unused]] const std::smatch &matches

using func_t = void(PARSE_OPCODE_ARGS);
}; // namespace _N_PARSE

#define _N_PRNT		  prnt
#define _N_PRNT_UTILS _N_PARSE::utils

namespace _N_PRNT {
#define PRNT_OPCODE_ARGS                                                                           \
	[[maybe_unused]] const instrutction_t *instr, const frame_t *frame, const thread_dbg_data_t &dbg

using func_t = void(PRNT_OPCODE_ARGS);
}; // namespace _N_PRNT

///////////////////// OPCODE_MACROS ////////////////////////////

#define _N_ARGS		  args
#define _N_ARGS_UTILS _N_ARGS::utils

#define STRINGIFY(x) #x
#define nameof(x)	 STRINGIFY(x)

#define REQUIRE_PARSE(macro)                                                                       \
	namespace _N_ARGS {                                                                            \
	const std::vector<operand_t> &macro();                                                         \
	}                                                                                              \
	namespace _N_PARSE {                                                                           \
	func_t macro;                                                                                  \
	}

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

///////////////////// IMPL ////////////////////////////

struct instrutction_t {
	constexpr static size_t args_per_instr = 2;
	_N_EXEC::func_t *action;
	unit arg[args_per_instr];

	instrutction_t(_N_EXEC::func_t *action) : action{action} {
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

// template <> struct std::formatter<code_pos_t> : std::formatter<std::string> {
// 	auto format(const code_pos_t &pos, auto &ctx) {
// 		std::string ans = std::format("[{}:{}]", pos.func_id, pos.pos);
// 		return std::formatter<std::string>::format(ans, ctx);
// 	}
// };

struct flag_data {
	bool were_equal = false;
	bool first_was_bigger = false;
};

struct frame_t {
	const instrutction_t *instr;
	unit *stack_start;
	unit *stack_head;
	flag_data flags;
	func_id_t cur_func_id;
};

struct function_t {
	code_block body;
	uint64_t no_of_args;
	uint64_t res_size;
	mutable std::vector<unit> regs;
};

namespace std {
template <> struct hash<code_pos_t> {
	constexpr size_t operator()(const code_pos_t &code_pos) const {
		return std::hash<func_id_t>{}(code_pos.func_id) ^ code_pos.pos;
	}
};
} // namespace std

#define enforce_stop(code, ...)                                                                    \
	throw dbg_require_stop{.inform = [= __VA_OPT__(, __VA_ARGS__)]() { code; }};

struct dbg_require_stop {
	std::function<void()> inform;
};

struct io_handler_t {
	enum class INPUT_INTERFACE {
		SINGLE_LINE_OR_PREV,
		UNTILL_SUCCESS,
	};

	INPUT_INTERFACE state;
	std::shared_ptr<std::string> entire_content;
	std::vector<std::string> last_nonwhite_lines;

	std::stringstream stream;
	std::streampos stream_start;
	int last_content_size = 0;

	bool was_line = false;
	bool success = false;

	io_handler_t(INPUT_INTERFACE mode) :
		entire_content(std::make_shared<std::string>("")),
		state(mode) {}

	io_handler_t(io_handler_t &oth) :
		state{oth.state},
		last_nonwhite_lines{oth.last_nonwhite_lines},
		entire_content{oth.entire_content},
		stream{*oth.entire_content.get()},
		stream_start(stream.tellg()) {
		stream.ignore(oth.stream.tellg() - oth.stream_start);
	}

	io_handler_t(io_handler_t &&) = default;

	static bool all_white(const std::string &str) {
		return std::all_of(str.begin(), str.end(), [](char c) { return bool(std::isspace(c)); });
	}

	void get_common_line(std::string &line_buf, char delim = '\n') {
		auto &str = *entire_content.get();

		if (str.size() == last_content_size) {
			CORE_ASSERT(std::getline(std::cin, line_buf, delim), "Expeected a line");
			line_buf += delim;
			str += line_buf;
			last_content_size = str.size();

			return;
		}

		std::string tmp = "";

		while (last_content_size < str.size()) {
			char c = str[last_content_size++];

			tmp += c;
			if (c == delim) {
				break;
			}
		}

		line_buf = tmp;
	}

	void handle_single_line(auto &elem) {
		if (was_line == true) {
			success = false;
			return;
		}

		std::string tmp;
		get_common_line(tmp);
		was_line = true;

		if (!all_white(tmp)) {
			last_nonwhite_lines.push_back(tmp);
		}

		if (last_nonwhite_lines.empty()) {
			success = false;
			return;
		}

		stream.clear();
		stream.str(last_nonwhite_lines.back());

		success = bool(stream >> elem);
	}

	void handle_until_success(auto &elem) {
		if (stream >> elem) {
			success = true;
			return;
		}

		std::string tmp;
		while (!(stream >> elem)) {
			if (!stream.eof()) {
				success = false;
				return;
			}

			get_common_line(tmp);

			stream.clear();
			stream.str(tmp);
		}
		last_nonwhite_lines.push_back(tmp);
		success = true;
	}

	io_handler_t &operator>>(auto &elem) {
		success = false;
		if (stream >> elem) {
			success = true;
			return *this;
		}

		switch (state) {
		case INPUT_INTERFACE::SINGLE_LINE_OR_PREV:
			handle_single_line(elem);
			break;
		case INPUT_INTERFACE::UNTILL_SUCCESS:
			handle_until_success(elem);
			break;
		}

		return *this;
	}

	void discard_remaining() {
		stream.clear();
		stream.str("");
		was_line = false;
	}

	operator bool() const { return success; }
};

namespace _N_PRNT_UTILS {
static const std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> dict = {
#define PRNT_ENTRY(macro) {&_N_EXEC::macro, &_N_PRNT::macro},
	APPLY_TO_SIMPLE(PRNT_ENTRY) APPLY_TO_JUMP(PRNT_ENTRY)};
} // namespace _N_PRNT_UTILS

struct thread_dbg_data_t {

	struct func_dbg_data_t {
		reg_id_map registers;
		labl_id_map labels;
		labl_map label_positions;

		std::string get_reg_name(unit id) const {
			auto opt = registers.by_second(id);
			CORE_ASSERT(opt.has_value(), "Why are you trying to print a non-existant function?!");
			return opt.value();
		}
	};

	std::unordered_map<func_id_t, func_dbg_data_t> func_data;
	biject_map_t<breakpoints_id, code_pos_t> breakpoints;
	func_id_map function_names;
	io_handler_t cli_io{io_handler_t::INPUT_INTERFACE::SINGLE_LINE_OR_PREV};

	std::optional<breakpoints_id> hit_breakpoint(const code_pos_t &position) {
		return breakpoints.by_second(position);
	}

	breakpoints_id set_breakpoint(const code_pos_t &position) {
		static breakpoints_id id = 0;
		CORE_ASSERT(breakpoints.emplace_first(++id, position).first);
		return id;
	}

	void stop_exec(code_pos_t position, thread_t &thread) {
		if (auto breakpoint = hit_breakpoint(position); breakpoint.has_value()) {
			enforce_stop(std::println("Hit breakpoint {}", breakpoint.value());)
		}
	}

	void handle_result(unit res) {
		if (res != 0 && res != 5318008) {
			enforce_stop(std::println("Execution returned error-result {}", res);)
		}
		throw res;
	}

	std::string get_func_name(func_id_t id) const {
		auto val = function_names.by_second(id);
		CORE_ASSERT(val.has_value(), "Why are you trying to print a non-existant function?!");
		return val.value();
	}
};

struct heap_t {
	std::unique_ptr<unit[]> content;
	std::set<std::pair<unit, unit>> allocated;
	const size_t size;

	heap_t(size_t alloc) :
		content(std::make_unique<unit[]>(alloc)),
		allocated{},
		size{alloc} {CORE_ASSERT(content, "Couldn't successfully allocate buffer for heap!")}

		unit allocate(unit chunk_size) {
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

	void deallocate(unit ptr) {
		auto it = allocated.lower_bound({ptr, 0});

		CORE_ASSERT(it != allocated.end() && it->first == ptr,
					"address {} was not used for allocation", ptr);

		allocated.erase(it);
		return;
	}

	bool is_in_allocated(unit ptr) {
		if (ptr >= size) {
			return false;
		}
		auto it = allocated.upper_bound({ptr, -1});

		return (it != allocated.begin() && (--it)->first <= ptr && ptr < it->first + it->second);
	}

	std::optional<unit> read(unit ptr) {
		return (is_in_allocated(ptr) ? content[ptr] : std::optional<unit>{});
	}

	bool write(unit ptr, unit val) {
		if (!is_in_allocated(ptr)) {
			return false;
		}

		content[ptr] = val;

		return true;
	}
};

struct memory_space {
	const size_t MEM_STACK_SIZE;
	const size_t CALL_STACK_SIZE;
	const size_t HEAP_SIZE;

	std::unique_ptr<unit[]> MEM_STACK;
	std::unique_ptr<frame_t[]> CALL_STACK;
	heap_t HEAP;
	io_handler_t IO;

	memory_space(size_t MEM_STACK_SIZE = 10, size_t CALL_STACK_SIZE = 1000,
				 size_t HEAP_SIZE = 2048) :
		MEM_STACK_SIZE(MEM_STACK_SIZE),
		CALL_STACK_SIZE(CALL_STACK_SIZE),
		HEAP_SIZE(HEAP_SIZE),
		MEM_STACK(std::make_unique<unit[]>(MEM_STACK_SIZE)),
		CALL_STACK(std::make_unique<frame_t[]>(CALL_STACK_SIZE)),
		HEAP(2048),
		IO(io_handler_t::INPUT_INTERFACE::UNTILL_SUCCESS) {
		CORE_ASSERT(MEM_STACK && CALL_STACK, "Couldn't allocate memory and call stacks");
	}
};

struct thread_t {
	func_map functions;
	func_id_t main_id;
	memory_space memory;

	bool running = false;

	auto init() {
		const instrutction_t *instr = functions.at(main_id).body.data();
		auto frame = memory.CALL_STACK.get();
		auto mem = &memory;

		frame->stack_head = memory.MEM_STACK.get();
		frame->stack_start = memory.MEM_STACK.get();
		frame->cur_func_id = main_id;

		memory.HEAP.allocated.clear();

		return std::make_tuple(instr, frame);
	}

	void start_execution() {
		auto [instr, frame] = init();
		try {
			exec(instr, frame);
		} catch (unit result) {
			std::print("The program has ended with a result {}\n", result);
		}
	}

	void exec(const instrutction_t *&instr, frame_t *&frame) {
		{
			instr += instr->action(instr, &memory, frame, &functions);
		}

		MUST_TAIL return exec(instr, frame);
	}

	INLINE void exec_single(const instrutction_t *&instr, frame_t *&frame, thread_dbg_data_t &dbg) {
		frame->instr = instr;

		try {
			instr += instr->action(instr, &memory, frame, &functions);
		} catch (unit result) {
			dbg.handle_result(result);
		}
	}

	void exec(const instrutction_t *&instr, frame_t *&frame, thread_dbg_data_t &dbg) {
		dbg.stop_exec(to_pos(instr, frame), *this);
		exec_single(instr, frame, dbg);

		MUST_TAIL return exec(instr, frame, dbg);
	}

	code_pos_t to_pos(const instrutction_t *const &instr, frame_t *const &frame) {
		auto func_id = frame->cur_func_id;
		return code_pos_t{
			.func_id = func_id,
			.pos = static_cast<size_t>(instr - functions[func_id].body.data()),
		};
	}

	bool was_undecided(thread_dbg_data_t &dbg) {
		std::cout << "Program is already running. Are you sure you want to execute this command? "
					 "(y / n)\n";
		io_handler_t loc_io{io_handler_t::INPUT_INTERFACE::SINGLE_LINE_OR_PREV};

		char ans;
		loc_io >> ans;
		std::cout << "[ans] " << ans << "\n";
		return (ans != 'y');
	}

	void safe_exec(const instrutction_t *&instr, frame_t *&frame, thread_dbg_data_t &dbg) {
		try {
			exec(instr, frame, dbg);
		} catch (unit &res) {
			std::println("program ended normally with {}", res);
			running = false;
		} catch (dbg_require_stop &stop) {
			stop.inform();
		}
	}

	void safe_next(const instrutction_t *&instr, frame_t *&frame, thread_dbg_data_t &dbg,
				   bool prnt = false) {
		try {
			exec_single(instr, frame, dbg);

			if (prnt) {
				show_pos(instr, frame, dbg, true);
			}
		} catch (unit res) {
			std::println("program ended normally with {}", res);
			running = false;
		} catch (dbg_require_stop &stop) {
			stop.inform();
		}
	}

	void start_debug_session(thread_dbg_data_t &dbg) {
		std::string input;
		auto [instr, frame] = init();

		while (true) {

			dbg.cli_io.discard_remaining();
			std::cout << "(debug) ";
			if (!(dbg.cli_io >> input)) {
				return;
			}
			std::cout << "[input] " << input << "\n";

			if (input == "run" || input == "r") {
				if (running && was_undecided(dbg)) {
					continue;
				}

				std::tie(instr, frame) = init();
				running = true;
				safe_exec(instr, frame, dbg);
				continue;
			}

			if (input == "start" || input == "s") {
				if (running && was_undecided(dbg)) {
					continue;
				}

				std::tie(instr, frame) = init();
				running = true;
				show_pos(instr, frame, dbg, true);

				continue;
			}

			if (input == "nexti" || input == "n") {
				if (!running) {
					std::cout << "No program is running\n";
					continue;
				}

				safe_next(instr, frame, dbg, true);
				continue;
			}

			if (input == "frame") {
				if (!running) {
					std::cout << "No program is running\n";
					continue;
				}

				show_pos(instr, frame, dbg, true);
				continue;
			}

			if (input == "backtrace" || input == "bt") {
				if (!running) {
					std::cout << "No program is running\n";
					continue;
				}

				show_call_stack(frame, dbg);
				continue;
			}

			if (input == "continue" || input == "c") {
				if (!running) {
					std::cout << "No program is running\n";
					continue;
				}

				safe_next(instr, frame, dbg);
				safe_exec(instr, frame, dbg);
				continue;
			}

			if (input == "stop") {
				if (!running) {
					std::cout << "No program is running\n";
					continue;
				}

				running = false;
				continue;
			}

			if (input == "quit" || input == "q") {
				return;
			}

			if (input == "break" || input == "b") {
				std::string func_name;
				uint64_t pos;

				if (!((dbg.cli_io >> func_name) && (dbg.cli_io >> pos))) {
					std::println("expected a func name and no of instr");
					continue;
				}

				auto opt = dbg.function_names.by_first(func_name);
				if (!opt.has_value()) {
					std::println("unknown function \"{}\"", func_name);
					continue;
				}

				code_pos_t code_pos = {.func_id = opt.value(), .pos = pos};

				breakpoints_id id = dbg.set_breakpoint(code_pos);
				std::println("New breakpoint {}", id);
				continue;
			}

			if (input == "info") {
				std::string spec;
				if (!(dbg.cli_io >> spec)) {
					std::println("expected a specifier to the region you are interested in");
					continue;
				}

				if (spec == "locals" || spec == "loc") {
					show_mem_stack(frame);
					continue;
				}

				if (spec == "registers" || spec == "reg") {
					std::string reg;
					if (!(dbg.cli_io >> reg)) {
						reg = "";
					}

					show_reg(reg, frame, dbg);
					
					continue;
				}

				if (spec == "heap") {
					unit ptr1, ptr2;
					if (!((dbg.cli_io >> ptr1) && (dbg.cli_io >> ptr2))) {
						std::println("expected a range for the heap analysis");
						continue;
					}

					show_heap(ptr1, ptr2);

					continue;
				}

			}

			std::println("unknown command: \"{}\"", input);
		}
	}

	void show_pos(const instrutction_t *const &instr, frame_t *const &frame, thread_dbg_data_t &dbg, bool detailed = false,
				  int idx = 0) {

		if (detailed) {
			std::cout << "\n";
			_N_PRNT_UTILS::dict.at(instr->action)(instr, frame, dbg);
		}

		auto [func_id, pos] = to_pos(instr, frame);
		auto func_name_opt = dbg.function_names.by_second(func_id);
		CORE_ASSERT(func_name_opt.has_value())
		std::println("#{:<5} func: {:<15} [id: {:<3}] instr no: {:<8}", idx, func_name_opt.value(), func_id,
					 pos);
	}

	void show_mem_stack(frame_t *frame) {
		unit *bottom = frame->stack_start;
		unit *top = frame->stack_head;

		while (top > bottom) {
			top--;
			std::println("[{:10}]", *top);
		}
		return;
	}

	void show_reg(const std::string& reg, frame_t* frame, thread_dbg_data_t& dbg) {
		auto func_id = frame->cur_func_id;

		auto it = dbg.func_data.find(func_id);
		CORE_ASSERT(it != dbg.func_data.end(), "There isn't function with id {} in dbg data", func_id);

		auto it2 = functions.find(func_id);
		CORE_ASSERT(it2 != functions.end(), "There isn't function with id {} in exec data", func_id);


		auto dbg_reg = it->second.registers;
		auto exec_reg = it2->second.regs;

		if (reg == "") {
			for (auto [reg_name, reg_id] : dbg_reg.map_1_to_2) {
				std::println("{:<10}: {:<10}", reg_name, exec_reg[reg_id]);
			}

			return;
		}

		auto it3 = dbg_reg.by_first(reg);
		if (!it3.has_value()) {
			std::print("There is no \"${}\" register in dbg symbols", reg);
			return;
		}

		auto reg_id = it3.value();
		std::println("{:<10}: {:<10}", reg, exec_reg[reg_id]);

	}

	void show_heap(unit ptr1, unit ptr2) {
		if (ptr1 >= memory.HEAP_SIZE || ptr2 >= memory.HEAP_SIZE) {
			std::println("Poiners {:<4} {:<4} are out of bounds - heap is of size {:<4}", ptr1,
						 ptr2, memory.HEAP_SIZE);
			return;
		}

		std::optional<unit> opt;
		std::string val;

		while (ptr1 <= ptr2) {
			opt = memory.HEAP.read(ptr1);
			val = opt.has_value() ? std::to_string(opt.value()) : "#";

			std::println("{:<3} {:<}", ptr1, val);
			ptr1++;
		}
	}

	void show_call_stack(frame_t *frame, thread_dbg_data_t &dbg) {
		auto bottom = memory.CALL_STACK.get();
		auto top = frame;
		while (frame >= bottom) {
			show_pos(frame->instr, frame, dbg, false, top - frame);
			frame--;
		}
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
		CORE_ASSERT(held.has_value(), "Using undeclared register {}", input);
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
		const auto &args = ::_N_ARGS::macro();                                                     \
		for (int i = 0; i < args.size(); i++) {                                                    \
			std::cout << " "                                                                       \
					  << _N_PRNT_UTILS::print_arg(instr->arg[i], frame->cur_func_id, args[i],      \
												  dbg);                                            \
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
		program.start_debug_session(debug_data);
	}

	return 0;
}
