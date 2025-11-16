#include <bits/stdc++.h>
#include <cstdint>
using namespace std;

using u64 = uint64_t;

u64 collatz(u64 n, u64 acc = 0) {
	return (n == 2) ? acc : collatz(
		(n % 2 == 0) ? (n / 2) : (3 * n + 1),  
		acc + 1
	);
}

int main() {
	u64 n;
	cin >> n;
	cout << collatz(n) << "\n";
	return 0;
}