#pragma once
#include "config.hpp"

struct instrutction_t;

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