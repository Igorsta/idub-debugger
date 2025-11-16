#include "definitions/func1.hpp"
#include "definitions/func2.hpp"


#ifndef HANDLED
	#define _DFAULT_HANDLED
	#define HANDLED(el)
#endif

HANDLED(FUNC1)
HANDLED(FUNC2)

#ifdef _DFAULT_HANDLED
	#undef _DFAULT_HANDLED
	#undef HANDLED
#endif