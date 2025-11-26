#pragma once
#include "debug.h"
#include <optional>
#include <unordered_map>

template <typename T1, typename T2>
struct biject_map_t {
	std::unordered_map<T1, T2> map_1_to_2;
	std::unordered_map<T2, T1> map_2_to_1;

	std::optional<T2> by_first(T1 key1) const {
		auto it = map_1_to_2.find(key1);
		return (it == map_1_to_2.end()) ? std::optional<T2>{} : it->second;
	}

	std::optional<T1> by_second(T2 key2) const {
		auto it = map_2_to_1.find(key2);
		return (it == map_2_to_1.end()) ? std::optional<T1>{} : it->second;
	}

	size_t size() {
		CORE_ASSERT(map_1_to_2.size() == map_2_to_1.size(),
					"bijection requires that both sets are equally big");
		return map_1_to_2.size();
	}

	std::pair<bool, T2> emplace_first(T1 key1, T2 val2) {
		CORE_ASSERT(map_2_to_1.find(val2) == map_2_to_1.end(),
					"Second value alread is in right set");

		auto it1 = map_1_to_2.find(key1);
		if (it1 != map_1_to_2.end()) {
			return std::make_tuple(false, it1->second);
		}

		map_1_to_2.emplace(key1, val2);
		map_2_to_1.emplace(val2, key1);
		return std::make_tuple(true, val2);
	}

	void clear() {
		map_1_to_2.clear();
		map_2_to_1.clear();
	}

	bool empty() { return (size() == 0); }
};
