#include "debug_data.hpp"
#include "thread.hpp"
#include "prnt.hpp"
#include <iostream>

std::optional<breakpoints_id> thread_dbg_data_t::hit_breakpoint(const code_pos_t &position) {
	return breakpoints.by_second(position);
}

breakpoints_id thread_dbg_data_t::set_breakpoint(const code_pos_t &position) {
	static breakpoints_id id = 0;
	CORE_ASSERT(breakpoints.emplace_first(++id, position).first);
	return id;
}

void thread_dbg_data_t::stop_exec(code_pos_t position, thread_t &thread) {
	if (auto breakpoint = hit_breakpoint(position); breakpoint.has_value()) {
		enforce_stop(std::println("Hit breakpoint {}", breakpoint.value());)
	}
}

void thread_dbg_data_t::handle_result(unit res) {
	if (res != 0 && res != 5318008) {
		enforce_stop(std::println("Execution returned error-result {}", res);)
	}
	throw res;
}

std::string thread_dbg_data_t::get_func_name(func_id_t id) const {
	auto val = function_names.by_second(id);
	CORE_ASSERT(val.has_value(), "Why are you trying to print a non-existant function?!");
	return val.value();
}

bool thread_dbg_data_t::was_undecided() {
	std::cout << "Program is already running. Are you sure you want to execute this command? "
					"(y / n)\n";
	io_handler_t loc_io{io_handler_t::INPUT_INTERFACE::SINGLE_LINE_OR_PREV};

	char ans;
	loc_io >> ans;
	std::cout << "[ans] " << ans << "\n";
	return (ans != 'y');
}

void thread_dbg_data_t::safe_exec(thread_t &thr) {
	try {
		thr.exec(*this);
	} catch (unit &res) {
		std::println("program ended normally with {}", res);
		running = false;
	} catch (dbg_require_stop &stop) {
		stop.inform();
	}
}

void thread_dbg_data_t::safe_next(thread_t &thr, bool prnt) {
	try {
		thr.exec_single(*this);
		thr.frame->instr = thr.instr;

		if (prnt) {
			show_pos(thr, true);
		}
	} catch (unit res) {
		std::println("program ended normally with {}", res);
		running = false;
	} catch (dbg_require_stop &stop) {
		stop.inform();
	}
}

void thread_dbg_data_t::start_debug_session(thread_t thr) {
	std::string input;
	thr.init();

	while (true) {

		cli_io.discard_remaining();
		std::cout << "(debug) ";
		if (!(cli_io >> input)) {
			return;
		}
		std::cout << "[input] " << input << "\n";

		if (input == "run" || input == "r") {
			if (running && was_undecided()) {
				continue;
			}

			thr.init();
			running = true;
			safe_exec(thr);
			continue;
		}

		if (input == "start" || input == "s") {
			if (running && was_undecided()) {
				continue;
			}

			thr.init();
			running = true;
			show_pos(thr, true);

			continue;
		}

		if (input == "nexti" || input == "n") {
			if (!running) {
				std::cout << "No program is running\n";
				continue;
			}

			safe_next(thr, true);
			continue;
		}

		if (input == "frame") {
			if (!running) {
				std::cout << "No program is running\n";
				continue;
			}

			show_pos(thr, true);
			continue;
		}

		if (input == "backtrace" || input == "bt") {
			if (!running) {
				std::cout << "No program is running\n";
				continue;
			}

			show_call_stack(thr);
			continue;
		}

		if (input == "continue" || input == "c") {
			if (!running) {
				std::cout << "No program is running\n";
				continue;
			}

			safe_next(thr);
			safe_exec(thr);
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

			if (!((cli_io >> func_name) && (cli_io >> pos))) {
				std::println("expected a func name and no of instr");
				continue;
			}

			auto opt = function_names.by_first(func_name);
			if (!opt.has_value()) {
				std::println("unknown function \"{}\"", func_name);
				continue;
			}

			code_pos_t code_pos = {.func_id = opt.value(), .pos = pos};

			breakpoints_id id = set_breakpoint(code_pos);
			std::println("New breakpoint {}", id);
			continue;
		}

		if (input == "info") {
			std::string spec;
			if (!(cli_io >> spec)) {
				std::println("expected a specifier to the region you are interested in");
				continue;
			}

			if (spec == "locals" || spec == "loc") {
				show_mem_stack(thr.frame);
				continue;
			}

			if (spec == "registers" || spec == "reg") {
				std::string reg;
				if (!(cli_io >> reg)) {
					reg = "";
				}

				show_reg(reg, thr.call_stack_top(), thr);

				continue;
			}

			if (spec == "heap") {
				unit ptr1, ptr2;
				if (!((cli_io >> ptr1) && (cli_io >> ptr2))) {
					std::println("expected a range for the heap analysis");
					continue;
				}

				show_heap(ptr1, ptr2, thr);

				continue;
			}
		}

		std::println("unknown command: \"{}\"", input);
	}
}

void thread_dbg_data_t::show_pos(thread_t &thr, bool detailed, int idx,
				std::optional<code_pos_t> code_pos) {

	if (!code_pos.has_value()) {
		code_pos.emplace(thr.call_stack_top());
	}
	auto [func_id, pos] = code_pos.value();

	if (false && detailed) {
		std::cout << "\n";

		_N_PRNT_UTILS::dict().at(func_data[func_id].instr_type[pos])(code_pos.value(), thr,
																	*this);
	}

	auto func_name_opt = function_names.by_second(func_id);
	if (!func_name_opt.has_value()) {
		CORE_ASSERT(func_name_opt.has_value())
	}
	std::println("#{:<5} func: {:<15} [id: {:<3}] instr no: {:<8}", idx, func_name_opt.value(),
					func_id, pos);
}

void thread_dbg_data_t::show_mem_stack(frame_t *frame) {
	unit *bottom = frame->stack_start;
	unit *top = frame->stack_head;

	while (top > bottom) {
		top--;
		std::println("[{:10}]", *top);
	}
	return;
}

void thread_dbg_data_t::show_reg(const std::string &reg, code_pos_t pos, thread_t &thr) {
	auto [func_id, _] = pos;

	auto it = func_data.find(func_id);
	CORE_ASSERT(it != func_data.end(), "There isn't function with id {} in dbg data", func_id);

	auto it2 = thr.functions.find(func_id);
	CORE_ASSERT(it2 != thr.functions.end(), "There isn't function with id {} in exec data",
				func_id);

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

void thread_dbg_data_t::show_heap(unit ptr1, unit ptr2, thread_t &thr) {
	if (ptr1 >= thr.memory.HEAP_SIZE || ptr2 >= thr.memory.HEAP_SIZE) {
		std::println("Poiners {:<4} {:<4} are out of bounds - heap is of size {:<4}", ptr1,
						ptr2, thr.memory.HEAP_SIZE);
		return;
	}

	std::optional<unit> opt;
	std::string val;

	while (ptr1 <= ptr2) {
		opt = thr.memory.HEAP.read(ptr1);
		val = opt.has_value() ? std::to_string(opt.value()) : "#";

		std::println("{:<3} {:<}", ptr1, val);
		ptr1++;
	}
}

void thread_dbg_data_t::show_call_stack(thread_t &thr) {
	int call_stack_size = thr.no_of_frames();
	for (int i = 0; i < call_stack_size; i++) {
		show_pos(thr, false, i, thr.call_stack_top(i));
	}
}
