#pragma once
#include <utility>

#if defined(__clang__)
	#define PUSH_DIAGNOSTIC _Pragma("clang diagnostic push")
	#define NO_SHADOW       _Pragma("clang diagnostic ignored \"-Wshadow-all\"")
	#define POP_DIAGNOSTIC  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
	#define PUSH_DIAGNOSTIC _Pragma("GCC diagnostic push")
	#define NO_SHADOW                                        \
		_Pragma("GCC diagnostic ignored \"-Wshadow=local\"") \
			_Pragma("GCC diagnostic ignored \"-Wshadow=compatible-local\"")
	#define POP_DIAGNOSTIC _Pragma("GCC diagnostic pop")
#endif

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
	)::detail::DeferHelper _{ [&]() noexcept -> void { code; } };         \
	POP_DIAGNOSTIC
#if __cplusplus >= 202'600L
	#warning "Remove the pragmas above when upgrading to C++26"
#endif
