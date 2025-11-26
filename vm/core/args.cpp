#include "args.hpp"
#include "opcodes/opcode.hpp"

namespace _N_ARGS_UTILS {
	#define NAME_BIND(code) {nameof(code), code()},
	std::unordered_map<std::string, const std::vector<operand_t> &> const& str_to_arg() {
	static std::unordered_map<std::string, const std::vector<operand_t> &> inner = {
		APPLY_TO_ALL(NAME_BIND)};

	return inner;
}

}; // namespace _N_ARGS_UTILS