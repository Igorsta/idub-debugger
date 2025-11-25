#pragma once

#include "config.hpp"
#include "debug.h"
#include <memory>
#include <set>

struct heap_t {
	std::unique_ptr<unit[]> content;
	std::set<std::pair<unit, unit>> allocated;
	const size_t size;

	heap_t(size_t alloc);

	unit allocate(unit chunk_size);

	void deallocate(unit ptr);

	bool is_in_allocated(unit ptr);

	std::optional<unit> read(unit ptr);

	bool write(unit ptr, unit val);
};