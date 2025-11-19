
#include <algorithm>
#include <cassert>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <print>
#include <sstream>
#include <tuple>
#include <vector>

#define ASSERT_INPUT(cond, ...)                                                                    \
	if (!(cond)) {                                                                                 \
		throw io_error_t(std::format(__VA_ARGS__));                                                \
	}

struct io_error_t {
	std::string msg;
};

struct io_handler_t {
	enum class INPUT_INTERFACE {
		SINGLE_LINE_OR_PREV,
		UNTILL_SUCCESS,
	};

	INPUT_INTERFACE state;
	std::shared_ptr<std::string> entire_content;
	std::vector<std::string> last_nonwhite_lines;

	std::stringstream stream;
	std::streampos stream_start;
	int last_content_size = 0;

	bool was_line = false;
	bool success = false;

	io_handler_t(INPUT_INTERFACE mode) :
		entire_content(std::make_shared<std::string>("")),
		state(mode) {}

	io_handler_t(io_handler_t &oth) :
		state{oth.state},
		last_nonwhite_lines{oth.last_nonwhite_lines},
		entire_content{oth.entire_content},
		stream{*oth.entire_content.get()},
		stream_start(stream.tellg()) {
		stream.ignore(oth.stream.tellg() - oth.stream_start);
	}

	io_handler_t(io_handler_t &&) = default;

	static bool all_white(const std::string &str) {
		return std::all_of(str.begin(), str.end(), [](char c) { return bool(std::isspace(c)); });
	}

	void get_common_line(std::string &line_buf, char delim = '\n') {
		auto &str = *entire_content.get();

		if (str.size() == last_content_size) {
			ASSERT_INPUT(std::getline(std::cin, line_buf, delim), "Expeected a line");
			line_buf += delim;
			str += line_buf;
			last_content_size = str.size();

			return;
		}

		std::string tmp = "";

		while (last_content_size < str.size()) {
			char c = str[last_content_size++];

			tmp += c;
			if (c == delim) {
				break;
			}
		}

		line_buf = tmp;
	}

	void handle_single_line(auto &elem) {
		if (was_line == true) {
			success = false;
			return;
		}

		std::string tmp;
		get_common_line(tmp);
		was_line = true;

		if (!all_white(tmp)) {
			last_nonwhite_lines.push_back(tmp);
		}

		if (last_nonwhite_lines.empty()) {
			success = false;
			return;
		}

		stream.clear();
		stream.str(last_nonwhite_lines.back());

		success = bool(stream >> elem);
	}

	void handle_until_success(auto &elem) {
		if (stream >> elem) {
			success = true;
			return;
		}

		std::string tmp;
		while (!(stream >> elem)) {
			get_common_line(tmp);

			stream.clear();
			stream.str(tmp);
		}
		last_nonwhite_lines.push_back(tmp);
		success = true;
	}

	io_handler_t &operator>>(auto &elem) {
		success = false;
		if (stream >> elem) {
			success = true;
			return *this;
		}

		switch (state) {
		case INPUT_INTERFACE::SINGLE_LINE_OR_PREV:
			handle_single_line(elem);
			break;
		case INPUT_INTERFACE::UNTILL_SUCCESS:
			handle_until_success(elem);
			break;
		}

		return *this;
	}

	void discard_remaining() {
		stream.clear();
		stream.str("");
		was_line = false;
	}

	operator bool() const { return success; }
};

struct io_state_handler_t {
	io_handler_t &io;
	io_handler_t::INPUT_INTERFACE prev_state;
	std::string prev_str;

	io_state_handler_t(io_handler_t &io_to_guard, io_handler_t::INPUT_INTERFACE new_state) :
		io{io_to_guard},
		prev_state(io_to_guard.state) {
		io_to_guard.state = new_state;
		io_to_guard.discard_remaining();
	}

	~io_state_handler_t() { io.state = prev_state; }
};

io_handler_t global(io_handler_t::INPUT_INTERFACE::UNTILL_SUCCESS);
io_handler_t dbg_io(io_handler_t::INPUT_INTERFACE::SINGLE_LINE_OR_PREV);

std::string get_input() {

	std::string input;

	std::cout << "(debug) ";
	dbg_io >> input;

	std::cout << "[input] " << input << "\n";

	return input;
}

void run() {
	uint64_t numb;
	global >> numb;
	std::cout << 2 * numb << "\n";

	return;
}

void run_2(io_handler_t &forked) {
	uint64_t numb;
	forked >> numb;
	std::cout << 2 * numb << "\n";

	return;
}

int main() {
	io_handler_t copy = global;

	std::string s;
	while (true) {
		dbg_io.discard_remaining();
		s = get_input();

		if (s == "r1") {
			run();
			continue;
		}
		if (s == "r2") {
			run_2(copy);
			continue;
		}

		if (s == "c") {
			continue;
		}

		break;
	}

	return 0;
}