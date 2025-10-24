#pragma once

#if defined(__clang__) && __clang_major__ >= 15
	#define INLINE [[clang::always_inline]] inline
#elif defined(__GNUG__) && __GNUG__ >= 6
	#define INLINE [[gnu::always_inline]] inline
#else
	#define INLINE
	#warning "Using INLINE without a support from compiler"
#endif