#include <cstddef>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <type_traits>
#include "musttail.h"
#include "inline.h"

using exec_func_t = void(size_t);
constexpr size_t STEPS = 1e11;

int rare_event_1_over_n2(uint64_t n) {
	return (n % STEPS == 0) ? 1 : 0;
}

INLINE void compute(size_t &collatz) {
	collatz = (collatz % 2 == 0) ? (collatz / 2) : (collatz * 3 + 1);
}

size_t global_collatz;

using exec_func_opt = std::optional<size_t>(size_t&);

std::optional<size_t> compute_opt(size_t &collatz) {
	compute(collatz);
	return 1;
}

std::optional<size_t> terminate_opt(size_t &collatz) {
	std::cout << collatz << "\n";
	return {};
}

exec_func_opt* opt_funcs[] = {&compute_opt, &terminate_opt};

void by_unlikely(size_t instr) {
	{
		int chosen = rare_event_1_over_n2(instr);
		auto val = opt_funcs[chosen](global_collatz);

		if (!val.has_value()) [[unlikely]] {
			return;
		}

		instr += val.value();
	}

	MUST_TAIL return by_unlikely(instr);
}

void by_optional(size_t instr) {
	{
		int chosen = rare_event_1_over_n2(instr);
		auto val = opt_funcs[chosen](global_collatz);

		if (!val.has_value()) {
			return;
		}

		instr += val.value();
	}

	MUST_TAIL return by_optional(instr);
}

using exec_func_throw = size_t(size_t&);

size_t compute_throw(size_t &collatz) {
	compute(collatz);
	return 1;
}

size_t terminate_throw(size_t &collatz) {
	std::cout << collatz << "\n";
	throw std::runtime_error("we finished");
}

exec_func_throw* throw_funcs[] = {&compute_throw, &terminate_throw};

void by_throw(size_t instr) {
	{
		int chosen = rare_event_1_over_n2(instr);
		instr += throw_funcs[chosen](global_collatz);
	}

	MUST_TAIL return by_throw(instr);
}

void by_throw_save(size_t instr) {
	try
	{
		int chosen = rare_event_1_over_n2(instr);
		instr += throw_funcs[chosen](global_collatz);
	} catch (std::runtime_error e) {
		std::cout << "Runtime-info about exception: " << e.what() << "\n";
		throw;
	}

	MUST_TAIL return by_throw(instr);
}


struct wrapper {
	using func_t = void(wrapper *, size_t, size_t&);
	func_t* action;
};

void compute_dedicated(wrapper * ptr, size_t instr, size_t &collatz) {
	compute(collatz);
	MUST_TAIL return ptr[rare_event_1_over_n2(instr)].action(&ptr[rare_event_1_over_n2(instr)], instr + 1, collatz);
}

void terminate_dedicated(wrapper * ptr, size_t instr, size_t &collatz) {
	std::cout << collatz << "\n";
	return;
}

wrapper dedicated_funcs[] = {
	wrapper{.action = &compute_dedicated},
	wrapper{.action = &terminate_dedicated},
};

void by_dedicated(size_t instr) {
	 return dedicated_funcs[rare_event_1_over_n2(instr)].action(&dedicated_funcs[rare_event_1_over_n2(instr)], instr + 1, global_collatz);
} 

template <auto func> void start_execution() {
	global_collatz = 42;

	if constexpr (func == by_throw || func == by_throw_save) {
		try {
			func(1);
		} catch (std::runtime_error e) {
			return;
		}

		return;
	}

	func(1);
	return;	
}

template <auto func> void measure() {
	auto start = std::chrono::high_resolution_clock::now();
	
	func();
	
	auto end = std::chrono::high_resolution_clock::now();
	
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	
	std::cout << "Time: " << duration.count() << " microseconds\n";
}


int main(int argc, char *argv[]) {

	// for (int i = 1; rare_event_1_over_n2(i) == 0; i++) {
	// 	std::cout << i << "\n";
	// }

	measure<start_execution<by_optional>>();
	measure<start_execution<by_unlikely>>();

	measure<start_execution<by_throw_save>>();	// fast
	measure<start_execution<by_throw>>();
	measure<start_execution<by_dedicated>>();
}