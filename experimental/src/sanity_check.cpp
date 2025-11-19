// This file serves as sanity check for the author
// together with site CompilerExplorer
// some of th code was generated/inspired by AI
#include "musttail.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ios>
#include <limits>
#include <memory>
#include <string>

template <std::convertible_to<int>... Args> auto sum(Args... args) { return (args + ...); }

//////////////// PACK INDEXING WORKS ON RUNTIME //////////////////////
// template <std::convertible_to<int>... Args>
// auto sum2(Args... args) {
// 	int ans = 0;

// 	for (int i = 0; i < sizeof...(Args); i++) {
// 		ans += args...[i];
// 	}

// 	return ans;
// }

template <std::convertible_to<int>... Args> auto sum3(Args... args) {
	std::array<int, sizeof...(Args)> buffer{args...};

	int ans = 0;
	for (int i = 0; i < buffer.size(); i++) {
		ans += buffer[i];
	}

	return ans;
}

int switch_func(int x) {
	switch (x) {
	case 21:
		return 37;
	default:
		return 0;
	};
}

int collatz(size_t x, size_t acc = 0) {
	if (x == 1) {
		return acc;
	}
	MUST_TAIL return collatz(x % 2 == 0 ? x / 2 : 3 * x + 1, acc + 1);
}

using func_type = void(int);

#define FUNC_NAME  my_cool_func_name
#define FUNC_NAME2 my_cool_func_name

static void FUNC_NAME() {

};

namespace element {
static func_type FUNC_NAME2;
}

// void element::my_cool_func_name(int arg) { std::cout << "arg: " << arg << "\n"; }

void element::FUNC_NAME2(int arg) { std::cout << "arg: " << arg << "\n"; }

//////////////////////////// MACRO FUN ////////////////////////////

// #define STRINGIFY(x) #x
// #define TOSTRING(x) STRINGIFY(x)

// // Example usage with direct expansion
// #define EXAMPLE_MACRO "Hello, World"
// #define ANOTHER_MACRO 42 + 1
// std::cout << STRINGIFY(EXAMPLE_MACRO) << std::endl;    // Output: "EXAMPLE_MACRO"
// std::cout << TOSTRING(EXAMPLE_MACRO) << std::endl;     // Output: "\"Hello, World\""
// std::cout << TOSTRING(ANOTHER_MACRO) << std::endl;	   // Output: "42 + 1"

/////////////////////////// REAL USE ////////////////////////////////

#define STCK_INIT	 init
#define STCK_DEINIT	 deinit
#define REG_TO_STCK	 reg_to_stk
#define STCK_TO_REG	 stk_to_reg
#define REG_TO_RVAL	 reg_to_retval
#define REG_TO_REG	 reg_to_reg
#define INPUT_TO_REG input_reg
#define OUTPUT_REG	 output_reg
#define RETURN		 ret
#define EXIT_PROG	 exit
#define CALL		 call

// Step 1: normal stringification
#define STRINGIFY(x) #x

// Step 2: expand first, then stringify
#define nameof(x) STRINGIFY(x)

enum class EXEC_MODES {
	TAIL,
	STACK,
};

template <EXEC_MODES mode = EXEC_MODES::TAIL> uint64_t factorial(uint64_t n, uint64_t acc = 1) {
	if (n == 0) {
		return acc;
	}

	if constexpr (mode == EXEC_MODES::TAIL) {
		MUST_TAIL return factorial(n - 1, n * acc);
	} else {
		return factorial(n - 1, n * acc);
	}
}

struct single {
	std::function<void()> action;

	single(auto el) : action(el) { action(); }
};

std::string get_input() {
	static std::string entire_line;
	static std::string last = "No previous command";
	std::string input;

	std::cout << "(debug) ";
	getline(std::cin, entire_line);

	if (entire_line != "") {
		std::stringstream cmd_line(entire_line);
		cmd_line >> input;
		last = input;
	}

	std::cout << "[input] " << last << "\n";

	return last;
}

struct io_handler_t {
	std::shared_ptr<std::string> entire_content;
	int pos;

	io_handler_t() : pos(0), entire_content(std::make_shared<std::string>("")) {}
	io_handler_t(io_handler_t &) = default;
	io_handler_t(io_handler_t &&) = default;

	void request_nonwhite_line() {
		auto &str = *entire_content.get();

		static std::string buff;

		auto it = str.begin() + pos;
		it = std::find_if(it, str.end(), [](char c) { return !bool(std::isspace(c)); });
		pos = it - str.begin();
		
		while (pos == str.size()) {
			getline(std::cin, buff, '\n');
			str += buff + '\n';
			it = std::find_if(str.begin() + pos, str.end(), [](char c) { return !bool(std::isspace(c)); });
			pos = it - str.begin();
		}

	}

	uint64_t read_num() {
		request_nonwhite_line();

		auto const& str = *entire_content.get();
		std::string ans = "";

		for (auto it = str.begin() + pos; it != str.end() && std::isdigit(*it); ++it) {
			ans += *it;
			pos++;
		}


		return std::stoull(ans);
	}
};

io_handler_t global;

void run() {

	auto numb = global.read_num();
	std::cout << 2 * numb << "\n";

	return;
}

void run_2(io_handler_t& forked) {

	auto numb = forked.read_num();
	std::cout << 2 * numb << "\n";

	return;
}

int main() {
	// std::cout << nameof(STCK_INIT) << '\n';
	// std::cout << nameof(REG_TO_STCK) << '\n';

	// std::cout << sum(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// // std::cout << sum2(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// std::cout << sum3(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// assert(switch_func(21) == 37 && switch_func(0) == 0);
	// collatz(837799);

	// element::my_cool_func_name(5);

	// factorial(5);
	// std::string line = "  init_imm $1 1";
	// auto reg = std::regex("\\s*init_imm\\s+\\$([A-Za-z0-9_]+),\\s+(\\d+)\\s*(?:#.*)?");
	// std::smatch matches;

	// std::cout << std::regex_match(line, matches, reg) << "\n";

	io_handler_t copy = global;

	std::string s;
	while (true) {
		s = get_input();

		if (s == "r1") {
			run();
			continue;
		}
		if (s == "r2") {
			run_2(copy);
			continue;
		}

		break;
	}


	return 0;
}