#pragma once

#include "opcode_def.hpp"
#include "requires.h"
#include "inline.h"
#include "../core/exec.hpp"
#include "../parse/prnt.hpp"
#include "../parse/parse.hpp"
#include "../config/args.hpp"

APPLY_TO_EXEC(REQUIRED_FOR_EXEC)
APPLY_TO_OTHER(REQUIRE_PARSE)