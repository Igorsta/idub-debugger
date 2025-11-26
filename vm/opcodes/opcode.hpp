#pragma once

#include "opcode_def.hpp"
#include "requires.h"
#include "inline.h"
#include "../core/exec.hpp"
#include "../core/prnt.hpp"
#include "../core/parse.hpp"
#include "../core/args.hpp"

APPLY_TO_EXEC(REQUIRED_FOR_EXEC)
APPLY_TO_OTHER(REQUIRE_PARSE)