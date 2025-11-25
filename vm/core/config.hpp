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
