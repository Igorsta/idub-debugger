#include "../config/config.hpp"
#include "debug.h"
#include "../parse/parse.hpp"
#include "thread.hpp"
#include "debug_data.hpp"
#include <filesystem>
#include <fstream>


//////////////////////////// PARSE SIMPLE OPCODE IMPL ////////////////////////////

std::string read_file(std::string file_name) {
	using _N_PARSE_UTILS::regex_func;

	std::ifstream file(std::filesystem::absolute(file_name));
	std::string content = {std::istreambuf_iterator<char>(file), {}};

	std::regex code_of_functions("(" + regex_func() + ")*\\s*");
	std::smatch matches;

	bool matched = std::regex_match(content, code_of_functions);
	CORE_ASSERT(matched, "Code of wrong format");

	return content;
}

//////////////////////////// OPCODE NORMAL VERSION ////////////////////////////

int main(int argc, const char *argv[]) {
	CORE_ASSERT(argc == 3, "the usage: {} [run|debug] <file name>", argv[0]);

	std::string mode = argv[1];
	std::string file = argv[2];
	std::string file_content = read_file(file);

	CORE_ASSERT(mode == "run" || mode == "debug", "Received an unknown command {}", mode);
	auto [program, debug_data] = _N_PARSE_UTILS::file_parse(file_content);

	if (mode == "run") {
		program.start_execution();
	} else {
		debug_data.start_debug_session(std::move(program));
	}

	return 0;
}
