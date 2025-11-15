#include <cstddef>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include "musttail.h"

using exec_func_t = void(size_t);
constexpr size_t STEPS = 1e9;

int rare_event_1_over_n2(uint64_t n) {
	return (n % STEPS == 0) ? 1 : 0;
}

size_t global_collatz;

using exec_func_opt = std::optional<size_t>(size_t&);

std::optional<size_t> compute_opt(size_t &collatz) {
	collatz = (collatz % 2 == 0) ? (collatz / 2) : (collatz * 3 + 1);

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
	collatz = (collatz % 2 == 0) ? (collatz / 2) : (collatz * 3 + 1);

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

template <exec_func_t func> void start_execution() {
	if constexpr (func == by_throw) {
		try {
			func(1);
		} catch (std::runtime_error e) {
			return;
		}

		return;
	}

	func(1);
}

using start_func_t = void();
template <start_func_t func> void measure() {
	auto start = std::chrono::high_resolution_clock::now();
	
	func();
	
	auto end = std::chrono::high_resolution_clock::now();
	
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	
	std::cout << "Time: " << duration.count() << " milliseconds\n";
}


int main(int argc, char *argv[]) {

	// for (int i = 1; rare_event_1_over_n2(i) == 0; i++) {
	// 	std::cout << i << "\n";
	// }

	measure<start_execution<by_optional>>();	// slowest
	measure<start_execution<by_unlikely>>();
	measure<start_execution<by_throw>>();		// fastest
	
}