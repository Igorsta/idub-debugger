#include "includes/func1.hpp"
#include "includes/func2.hpp"
#include <bits/stdc++.h>

using n1_t = int(int, int);
using n2_t = double(int, int);

std::vector<std::pair<n1_t *, n2_t *>> paired = {

#define HANDLED(el) {&n1::el, &n2::el},
	#include "utils/apply_to_all.hpp"
#undef HANDLED

	};

int main() {
	int a;
	a = 2;
	return 0;
}