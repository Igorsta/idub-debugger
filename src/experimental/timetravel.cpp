#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <stdint.h>
#include <sys/types.h>
#include <utility>
#include <vector>

//		here is thread
// |----[_______)-----------|
// 0	^LOW	^HIGH		1

using count_t = size_t;
using count_diff = size_t;
constexpr double LOW_BOUND = 0.5;
constexpr double HIGH_BOUND = 0.95;
constexpr count_t LAST_CHUNK = 1000;

class bidirect_thread;

class debug_thread {
  private:
	friend bidirect_thread;
	count_t count = 0; // number of executed instructions
	/**
	memory, functions, and others data ...
	*/

	bool is_in_interval(const debug_thread &previous, count_t target) const {
		count_t offset = previous.get_time();
		count_diff relative_count = (count - offset), relative_target = (target - offset);

		return (relative_count >= relative_target * LOW_BOUND) && (relative_count < relative_target * HIGH_BOUND);
	}

  public:
	debug_thread(count_t time) : count(time) {}
	count_t get_time() const { return count; }

	void execute_step(/* maybe some arguments ... */) {
		/**
		... some other fancy actions
		*/
		count++;
	}
};

class bidirect_thread {
  private:
	std::vector<debug_thread> subthreads = {debug_thread{0}};

	template <std::convertible_to<debug_thread> T> void add_thread(T &&el) {
		subthreads.push_back(std::forward<T>(el));
	}
	decltype(auto) last_thread() {
		return *subthreads.rbegin();
	}
	decltype(auto) last_thread() const {
		return *subthreads.rbegin();
	}
	void remove_last() {
		subthreads.erase(std::next(subthreads.rbegin()).base());
	}
	
	inline count_diff diff_of_last2() const {
		return last_thread().get_time() - subthreads[subthreads.size() - 2].get_time();
	}

	void assert_property() const {
		assert(subthreads.size());
		assert(subthreads[0].get_time() == 0);
		for (auto it = subthreads.begin(); it + 1 != subthreads.end(); ++it) {
			assert(it->get_time() < (it + 1)->get_time());
			assert(it->is_in_interval(*it, get_time()) || it + 1 != subthreads.end());
		}
		if (subthreads.size() > 1) {
			assert(diff_of_last2() < LAST_CHUNK);
		}
	}

  public:
	count_t get_time() const { return last_thread().get_time(); }

	void execute_step() {
		assert_property();

		count_t target = get_time() + 1;
		if (subthreads.size() > 1)
			for (auto it = subthreads.begin(); it + 2 != subthreads.end(); it++)
				if (!((it + 1)->is_in_interval(*it, target)))
					(it + 1)->execute_step();

		if (subthreads.size() == 1) {
			add_thread(debug_thread{0});
		}
		last_thread().execute_step();

		if (diff_of_last2() == LAST_CHUNK) {
			debug_thread old_last = last_thread();
			remove_last();

			add_thread(last_thread());
			for (count_diff i = 0; i < LAST_CHUNK * (LOW_BOUND + HIGH_BOUND) / 2; i++) {
				last_thread().execute_step();
			}

			add_thread(old_last);
		}
	}

	void revert(count_diff backwards) {
		assert_property();

		if (backwards >= get_time()) {
			while (subthreads.size() != 1) {
				remove_last();
			}

			return;
		}

		count_t target = get_time() - backwards;

		while (get_time() > target) {
			remove_last();
		}
	}

	void print_detail() const {
		for (int i = 0; i < subthreads.size(); i++) {
			std::cout << subthreads[i].get_time() << " ";
		}
		std::cout << "\n";
	}
};

int main() {
	bidirect_thread test{};

	for (int i = 0; i < (1 << 20); i++) {
		test.execute_step();
		std::cout << test.get_time() << ": ";
		test.print_detail();
		std::cout << "\n";
	}
}
