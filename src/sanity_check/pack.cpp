#include <bits/stdc++.h>

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

int main() {
	std::cout << sum(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	// std::cout << sum2(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	std::cout << sum3(2, 1, 3, 7, 6, 9, 4, 2, 0) << "\n";
	switch_func(21);
}