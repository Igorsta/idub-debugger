#include "includes/func1.hpp"

namespace n1 {
	int FUNC1(int a, int b) {
		return a + b;
	}
}

namespace n2 {
	double FUNC1(int a, int b) {
		return double(a) * b;
	}
}
