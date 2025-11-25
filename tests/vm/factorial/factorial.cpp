#include <bits/stdc++.h>
#include <cstdint>
using namespace std;

using u64 = uint64_t;

u64 fact(u64 n, u64 acc = 1) {
	return (n == 0) ? acc : fact(n - 1, acc * n);
}

int main() {
	u64 n;
	cin >> n;
	cout << fact(n) << "\n";
	return 0;
}