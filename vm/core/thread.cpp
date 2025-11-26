#include "thread.hpp"
#include "exec.hpp"
#include "musttail.h"
#include "debug_data.hpp"

void thread_t::init() {
	instr = functions.at(main_id).body.data();
	frame = memory.CALL_STACK.get();
	auto mem = &memory;

	frame->stack_head = memory.MEM_STACK.get();
	frame->stack_start = memory.MEM_STACK.get();
	frame->cur_func_id = main_id;
	frame->instr = instr;

	memory.HEAP.allocated.clear();

	return;
}

void thread_t::start_execution() {
	init();
	try {
		exec();
	} catch (unit result) {
		// std::print("The program has ended with a result {}\n", result);
	}
}

void thread_t::exec() {
	{
		instr += instr->action(instr, &memory, frame, &functions);
	}

	MUST_TAIL return exec();
}

void thread_t::exec_single(thread_dbg_data_t &dbg) {
	try {
		instr += instr->action(instr, &memory, frame, &functions);
	} catch (unit result) {
		dbg.handle_result(result);
	}
}

void thread_t::exec(thread_dbg_data_t &dbg) {
	frame->instr = instr;
	dbg.stop_exec(to_pos(instr, frame), *this);
	exec_single(dbg);

	MUST_TAIL return exec(dbg);
}

code_pos_t thread_t::to_pos(const instrutction_t *const &instr, frame_t *const &frame) {
	auto func_id = frame->cur_func_id;
	return code_pos_t{
		.func_id = func_id,
		.pos = static_cast<size_t>(instr - functions[func_id].body.data()),
	};
}

size_t thread_t::no_of_frames() { return frame - memory.CALL_STACK.get() + 1; }

code_pos_t thread_t::call_stack_top(size_t n) {
	CORE_ASSERT(n < no_of_frames());

	frame_t *ans_frame = &memory.CALL_STACK[no_of_frames() - 1 - n];

	return to_pos(ans_frame->instr, ans_frame);
}
