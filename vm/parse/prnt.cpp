#include "prnt.hpp"
#include "opcode_def.hpp"
#include "opcodes/opcode.hpp"

namespace _N_PRNT_UTILS {
std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> const &dict() {
	static std::unordered_map<_N_EXEC::func_t *, _N_PRNT::func_t *> inner = {
		#define PRNT_ENTRY(macro) {&_N_EXEC::macro, &_N_PRNT::macro},
	APPLY_TO_SIMPLE(PRNT_ENTRY) APPLY_TO_JUMP(PRNT_ENTRY)
	};
	
	return inner;
}

} // namespace _N_PRNT_UTILS
