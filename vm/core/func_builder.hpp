#pragma once

#include "debug_data.hpp"
#include "defer.h"
#include "../config/config.hpp"
#include "exec.hpp"

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

		std::vector<_N_EXEC::func_t *> type_of_instr;

		for (int i = 0; i < built_func_body.size(); i++) {
			type_of_instr.push_back(built_func_body[i].action);
		}

		return {
			.registers = reg_decl,
			.labels = labl_decl,
			.label_positions = label_pos,
			.instr_type = type_of_instr,
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