#include "includes/func2.hpp"

namespace n1 {
	int FUNC2(int a, int b) {
		return a - b;
	}
}

namespace n2 {
	double FUNC2(int a, int b) {
		return double(a) / b;
	}
}
