#include "heap.hpp"

heap_t::heap_t(size_t alloc) :
	content(std::make_unique<unit[]>(alloc)),
	allocated{},
	size{alloc} {CORE_ASSERT(content, "Couldn't successfully allocate buffer for heap!")}

unit heap_t::allocate(unit chunk_size) {
	auto it = allocated.begin();
	int start = 0;

	while (it != allocated.end() && start + chunk_size > it->first) {
		start = it->first + it->second;
		it++;
	}

	CORE_ASSERT(start + chunk_size <= size,
				"Failed to allocate chunk: {0} + {1} is bigger than {2}", start, chunk_size,
				size);
	allocated.insert({start, chunk_size});

	return start;
}

void heap_t::deallocate(unit ptr) {
	auto it = allocated.lower_bound({ptr, 0});

	CORE_ASSERT(it != allocated.end() && it->first == ptr,
				"address {} was not used for allocation", ptr);

	allocated.erase(it);
	return;
}

bool heap_t::is_in_allocated(unit ptr) {
	if (ptr >= size) {
		return false;
	}
	auto it = allocated.upper_bound({ptr, -1});

	return (it != allocated.begin() && (--it)->first <= ptr && ptr < it->first + it->second);
}

std::optional<unit> heap_t::read(unit ptr) {
	return (is_in_allocated(ptr) ? content[ptr] : std::optional<unit>{});
}

bool heap_t::write(unit ptr, unit val) {
	if (!is_in_allocated(ptr)) {
		return false;
	}

	content[ptr] = val;

	return true;
}
