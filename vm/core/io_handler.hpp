#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <vector>


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

	io_handler_t(INPUT_INTERFACE mode);

	io_handler_t(io_handler_t &oth);

	io_handler_t(io_handler_t &&) = default;

	static bool all_white(const std::string &str)  {
		return std::all_of(str.begin(), str.end(), [](char c) { return bool(std::isspace(c)); });
	}

	void get_common_line(std::string &line_buf, char delim = '\n');

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
			if (!stream.eof()) {
				success = false;
				return;
			}

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

	void discard_remaining();

	operator bool() const;
};
