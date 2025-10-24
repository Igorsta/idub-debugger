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

int main() {
	std::cout << sum(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// std::cout << sum2(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	std::cout << sum3(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	assert(switch_func(21) == 37 && switch_func(0) == 0);
	collatz(837799);
}