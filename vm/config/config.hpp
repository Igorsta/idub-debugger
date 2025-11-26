#pragma once

#include <cstdint>
#include <string>
#include "biject_map.hpp"

using func_id_t = uint64_t;
using unit = uint64_t;
using labl_id_t = uint64_t;
using reg_id_t = uint64_t;

using reg_id_map = biject_map_t<std::string, reg_id_t>;
using labl_id_map = biject_map_t<std::string, labl_id_t>;
using labl_map = std::unordered_map<labl_id_t, size_t>;
using func_id_map = biject_map_t<std::string, func_id_t>;

using breakpoints_id = uint64_t;

#define _N_ARGS		  opargs
#define _N_ARGS_UTILS _N_ARGS::utils


#define _N_EXEC		  exec
#define _N_EXEC_UTILS _N_EXEC::utils

#define _N_PARSE	   parse
#define _N_PARSE_UTILS _N_PARSE::utils

#define _N_PRNT		  prnt
#define _N_PRNT_UTILS _N_PRNT::utils
