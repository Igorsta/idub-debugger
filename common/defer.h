#pragma once
#include <utility>

namespace detail {
	template<typename ActionT>
	class DeferHelper final {
		ActionT action;

	public:
		DeferHelper(ActionT&& action): action(std::move(action)) {}

		~DeferHelper() noexcept { action(); }
	};
}

#define defer(code)                                                         \
	PUSH_DIAGNOSTIC _Pragma("GCC diagnostic ignored \"-Wc++26-extensions\"" \
	)::internal::DeferHelper _{ [&]() noexcept -> void { code; } };         \
	POP_DIAGNOSTIC
#if __cplusplus >= 202'600L
	#warning "Remove the pragmas above when upgrading to C++26"
#endif
