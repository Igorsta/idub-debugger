#pragma once
#include "config.hpp"

struct code_pos_t {
	func_id_t func_id;
	size_t pos;

	bool operator==(const code_pos_t &oth) const {
		return oth.func_id == func_id && pos == oth.pos;
	}
};

namespace std {
template <> struct hash<code_pos_t> {
	constexpr size_t operator()(const code_pos_t &code_pos) const {
		return std::hash<func_id_t>{}(code_pos.func_id) ^ code_pos.pos;
	}
};
} // namespace std