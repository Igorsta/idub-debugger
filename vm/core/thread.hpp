#pragma once
#include "code_pos.hpp"
#include "memory.hpp"
#include "function.hpp"

struct thread_dbg_data_t;

struct thread_t {
	func_map functions;
	func_id_t main_id;
	memory_space memory;

	const instrutction_t *instr;
	frame_t *frame;

	void init();

	void start_execution();

	void exec();

	void exec_single(thread_dbg_data_t &dbg);

	void exec(thread_dbg_data_t &dbg);

	code_pos_t to_pos(const instrutction_t *const &instr, frame_t *const &frame);

	size_t no_of_frames();

	code_pos_t call_stack_top(size_t n = 0);
};