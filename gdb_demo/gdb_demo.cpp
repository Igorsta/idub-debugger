#include <bits/stdc++.h>

int global_var = 512;

template <typename T>
T xorAdd(T a, T b) {
	a += 8;
	global_var *= 2;
	b -= 7;
	return a + b;
}

int boring_func() {
	return 1042;
}

int func() {
	static int el = 4;	// deklaracja statycznych zmiennych nie wyświteli się w gdb
	int func_local_var = boring_func();
	if (el != 0) {
		el += (el + 1) * (2 * el + 1) * el / 6;
	}
	return el--;
}

int main(int argc, const char *argv[]) {
	int ret = func();
	std::cout << ret << "\n";
	std::cout << xorAdd(ret, 2 * ret) << "\n";
}