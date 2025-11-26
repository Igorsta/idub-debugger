#pragma once

#include "exec.hpp"
#include "config/code_pos.hpp"
#include "../memory/io_handler.hpp"
#include <functional>

struct thread_t;

#define enforce_stop(code, ...)                                                                    \
	throw dbg_require_stop{.inform = [= __VA_OPT__(, __VA_ARGS__)]() { code; }};

struct dbg_require_stop {
	std::function<void()> inform;
};

struct failed_command {
	std::string why;
};

struct thread_dbg_data_t {
	struct func_dbg_data_t {
		reg_id_map registers;
		labl_id_map labels;
		labl_map label_positions;
		std::vector<_N_EXEC::func_t *> instr_type;

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

	bool running = false;
	bool active = true;

	std::optional<breakpoints_id> hit_breakpoint(const code_pos_t &position);

	breakpoints_id set_breakpoint(const code_pos_t &position);

	void stop_exec(code_pos_t position, thread_t &thread);

	void handle_result(unit res);

	void confirm_if_running();
	void ensure_is_running();

	void exec_cmd(const std::string& input, thread_t& thr);

	void handle_run(thread_t& thr);

	void handle_start(thread_t& thr);

	void handle_next(thread_t &thr);

	void handle_frame(thread_t &thr);

	void handle_backtrace(thread_t &thr);

	void handle_continue(thread_t &thr);

	void handle_stop(thread_t& thr);

	void handle_quit(thread_t& thr);

	void handle_break(thread_t& thr);

	void handle_info(thread_t &thr);

	void handle_help(thread_t &thr);	

	std::string get_func_name(func_id_t id) const;

	bool was_undecided();

	void safe_exec(thread_t &thr);

	void safe_next(thread_t &thr, bool prnt = false);

	void start_debug_session(thread_t thr);

	void show_pos(thread_t &thr, bool detailed = false, int idx = 0,
				  std::optional<code_pos_t> code_pos = {});

	void show_mem_stack(frame_t *frame);

	void show_reg(const std::string &reg, code_pos_t pos, thread_t &thr);

	void show_heap(unit ptr1, unit ptr2, thread_t &thr);

	void show_call_stack(thread_t &thr);
};