#pragma once
#include "heap.hpp"
#include "io_handler.hpp"
#include "frame.hpp"

struct memory_space {
	const size_t MEM_STACK_SIZE;
	const size_t CALL_STACK_SIZE;
	const size_t HEAP_SIZE;

	std::unique_ptr<unit[]> MEM_STACK;
	std::unique_ptr<frame_t[]> CALL_STACK;
	heap_t HEAP;
	io_handler_t IO;

	memory_space(size_t MEM_STACK_SIZE = 10, size_t CALL_STACK_SIZE = 1000,
				 size_t HEAP_SIZE = 2048) :
		MEM_STACK_SIZE(MEM_STACK_SIZE),
		CALL_STACK_SIZE(CALL_STACK_SIZE),
		HEAP_SIZE(HEAP_SIZE),
		MEM_STACK(std::make_unique<unit[]>(MEM_STACK_SIZE)),
		CALL_STACK(std::make_unique<frame_t[]>(CALL_STACK_SIZE)),
		HEAP(2048),
		IO(io_handler_t::INPUT_INTERFACE::UNTILL_SUCCESS) {
		CORE_ASSERT(MEM_STACK && CALL_STACK, "Couldn't allocate memory and call stacks");
	}

	memory_space(memory_space &&) = default;
};