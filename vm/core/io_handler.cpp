#include "io_handler.hpp"
#include "debug.h"
#include <memory>
#include <sstream>
#include <iostream>
#include <vector>


io_handler_t::io_handler_t(INPUT_INTERFACE mode) :
	entire_content(std::make_shared<std::string>("")),
	state(mode) {}

io_handler_t::io_handler_t(io_handler_t &oth) :
	state{oth.state},
	last_nonwhite_lines{oth.last_nonwhite_lines},
	entire_content{oth.entire_content},
	stream{*oth.entire_content.get()},
	stream_start(stream.tellg()) {
	stream.ignore(oth.stream.tellg() - oth.stream_start);
}


void io_handler_t::get_common_line(std::string &line_buf, char delim) {
	auto &str = *entire_content.get();

	if (str.size() == last_content_size) {
		CORE_ASSERT(std::getline(std::cin, line_buf, delim), "Expeected a line");
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


void io_handler_t::discard_remaining() {
	stream.clear();
	stream.str("");
	was_line = false;
}

io_handler_t::operator bool() const { return success; }

