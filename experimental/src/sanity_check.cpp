#include <bits/stdc++.h>
#include <cstddef>
#include "musttail.h"

template <std::convertible_to<int>... Args>
auto sum(Args... args) {
    return (args + ...);
}

//////////////// PACK INDEXING WORKS ON RUNTIME //////////////////////
// template <std::convertible_to<int>... Args>
// auto sum2(Args... args) {
// 	int ans = 0;

// 	for (int i = 0; i < sizeof...(Args); i++) {
// 		ans += args...[i];
// 	}

// 	return ans;
// }

template <std::convertible_to<int>... Args>
auto sum3(Args... args) {
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
	MUST_TAIL return collatz(
		x % 2  == 0 ? x / 2 : 3 * x + 1,
		acc + 1
	);
}

using func_type = void(int);

#define FUNC_NAME my_cool_func_name
#define FUNC_NAME2 my_cool_func_name

static void FUNC_NAME() {

};

namespace element {
	static func_type FUNC_NAME2;
}

// void element::my_cool_func_name(int arg) { std::cout << "arg: " << arg << "\n"; }

void element::FUNC_NAME2(int arg) {
	std::cout << "arg: " << arg << "\n";
}

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

#define STCK_INIT init
#define STCK_DEINIT deinit
#define REG_TO_STCK reg_to_stk
#define STCK_TO_REG stk_to_reg
#define REG_TO_RVAL reg_to_retval
#define REG_TO_REG reg_to_reg
#define INPUT_TO_REG input_reg
#define OUTPUT_REG output_reg
#define RETURN ret
#define EXIT_PROG exit
#define CALL call

// Step 1: normal stringification
#define STRINGIFY(x) #x

// Step 2: expand first, then stringify
#define nameof(x) STRINGIFY(x)


int main() {
    std::cout << nameof(STCK_INIT) << '\n';
	std::cout << nameof(REG_TO_STCK) << '\n';

	std::cout << sum(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// std::cout << sum2(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	std::cout << sum3(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	assert(switch_func(21) == 37 && switch_func(0) == 0);
	collatz(837799);

	element::my_cool_func_name(5);
}