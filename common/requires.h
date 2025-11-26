#pragma once

#define REQUIRE_ARGS(macro)                                                                        \
	namespace _N_ARGS {                                                                            \
	const std::vector<operand_t> &macro();                                                         \
	}

#define REQUIRED_FOR_EXEC(macro)                                                                   \
	REQUIRE_PARSE(macro)                                                                           \
	namespace _N_EXEC {                                                                            \
	func_t macro;                                                                                  \
	}                                                                                              \
	namespace _N_PRNT {                                                                            \
	func_t macro;                                                                                  \
	}

#define REQUIRE_PARSE(macro)                                                                       \
	REQUIRE_ARGS(macro)                                                                            \
	namespace _N_PARSE {                                                                           \
	func_t macro;                                                                                  \
	}