#pragma once

#include "func_builder.hpp"
#include "thread.hpp"

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