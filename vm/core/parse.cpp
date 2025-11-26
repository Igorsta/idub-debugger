#include "args.hpp"
#include "debug_data.hpp"
#include "thread.hpp"
#include "opcode_def.hpp"
#include "opcodes/opcode.hpp"


namespace _N_PARSE_UTILS {

void nop(PARSE_OPCODE_ARGS) { return; }

std::regex make_opcode_pattern(std::string op_name) {
	std::string ans = "\\s*" + op_name;

	using _N_ARGS_UTILS::str_to_arg;

	auto it = str_to_arg().find(op_name);
	CORE_ASSERT(it != str_to_arg().end(), "{} does not have defined operations", op_name);
	const auto &args = it->second;

	for (int i = 0; i < args.size(); i++) {
		ans += ("\\s+" + regex_arg_t(args[i]) + ",");
	}
	if (args.size()) {
		ans.pop_back(); // removing last "," if present
	}
	ans += "\\s*;";
	ans += "\\s*(?:#.*)?"; // komentarze są dozwolone

	return std::regex(ans);
}

void parse_line(const std::string &line, thread_builder_t &builder) {

	using namespace ::_N_PARSE;
	using _N_PARSE_UTILS::make_opcode_pattern;
	using _N_PARSE_UTILS::nop;

	static const std::unordered_map<std::string, std::pair<std::regex, func_t *>> str_to_instr = {
		{"#", {std::regex("\\s*(?:#.*)?"), nop}},
		{";", {std::regex("\\s*;\\s*(?:#.*)?"), nop}},

#define parse_pos(macro) {nameof(macro), {make_opcode_pattern(nameof(macro)), &macro}},

		APPLY_TO_ALL(parse_pos)
	};

	std::stringstream input(line);
	std::string prefix;
	input >> prefix;
	std::smatch matches;

	auto quick = str_to_instr.find(prefix);

	if (quick != str_to_instr.end() && std::regex_match(line, matches, quick->second.first)) {
		quick->second.second(builder, matches);
		return;
	}

	for (const auto &[_, entry] : str_to_instr) {
		const auto &[pattern, action] = entry;

		if (std::regex_match(line, matches, pattern)) {
			action(builder, matches);
			return;
		}
	}

	CORE_ASSERT(false, "Line \"{}\" could not be parsed", line);
}

std::tuple<thread_t, thread_dbg_data_t> file_parse(std::string &content) {
	thread_builder_t res{};

	static const std::regex func_re(regex_func());
	std::sregex_iterator func_begin(content.begin(), content.end(), func_re), func_end{};

	for (auto it = func_begin; it != func_end; it++) {
		const std::smatch &matches = *it;

		std::stringstream code(matches[4]);
		static std::string line;
		while (getline(code, line)) {
			parse_line(line, res);
		}
		res.commit_func(matches[1], stoull(matches[2]), stoull(matches[3]));
	}

	res.validate_prog();

	return std::make_tuple(res.make_thread(), res.debug_data);
}

} // namespace _N_PARSE_UTILS