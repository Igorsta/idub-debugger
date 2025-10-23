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

#if defined(__clang__) && __clang__ >= 13
#define MUST_TAIL [[clang::musttail]]
#elif defined(__GNUG__) && __GNUG__ >= 15
#define MUST_TAIL [[gnu::musttail]]
#else
#define MUST_TAIL
#warning "Using tailcalls without a support from compiler"
#endif

//		here is thread
// |----[_______)-----------|
// 0	^LOW	^HIGH		1

using count_t = uint64_t;
using count_diff = int64_t;
constexpr double LOW_BOUND = 0.5;
constexpr double HIGH_BOUND = 0.95;
constexpr count_t LAST_CHUNK = 1000;

class anachro_thread;

enum class pos_t {
	LEFT_BEHIND,
	LACK_OF_SPACE,
	BEFORE_LOW,
	AFTER_HIGH,
	EXACTLY_IN,
};

class debug_thread {
  private:
	friend class anachro_thread;
	count_t count = 0; // number of executed instructions
	/**
	memory, functions, and others data ...
	*/

	pos_t in_interval(const debug_thread &previous, count_t target) const {
		count_t offset = previous.get_time();
		count_diff relative_count = (count - offset), relative_target = (target - offset);

		if (relative_target < LAST_CHUNK) {
			return pos_t::LACK_OF_SPACE;
		}
		
		if (relative_count < relative_target * LOW_BOUND) {
			return pos_t::BEFORE_LOW;
		}

		if (relative_count >= relative_target * HIGH_BOUND) {
			return pos_t::AFTER_HIGH;
		}

		return pos_t::EXACTLY_IN;
	}

  public:
	debug_thread(count_t time) : count(time) {}
	count_t get_time() const { return count; }

	void execute(/* maybe some arguments ... */) {
		/**
		... some other fancy actions
		*/
		count++;
	}
};

class anachro_thread {
  private:
	std::vector<debug_thread> subthreads = {debug_thread{0}};

	template <std::convertible_to<debug_thread> T> void add_thread(T &&el) {
		subthreads.push_back(std::forward<T>(el));
	}

	decltype(auto) last_thread(this auto &&self) { return self.subthreads.back(); }

	void remove_last() { subthreads.pop_back(); }

	void assert_property() const {
		assert(subthreads.size());
		assert(subthreads[0].get_time() == 0);
		for (auto it = subthreads.begin(); it + 1 != subthreads.end(); ++it) {
			assert(it->get_time() < (it + 1)->get_time());
			assert((it + 1)->in_interval(*it, get_time()) == pos_t::EXACTLY_IN);
		}
		if (subthreads.size() > 1) {
			assert(last_thread().get_time() - subthreads[subthreads.size() - 2].get_time() < LAST_CHUNK);
		}
	}

	void restore_property(count_t target, int idx = 0) {
		if (subthreads.size() <= 1 || idx + 1 >= subthreads.size()) {
			return;
		}

		debug_thread &self = subthreads[idx];
		debug_thread &son = subthreads[idx + 1];
		pos_t rel_pos = son.in_interval(self, target);

		switch (rel_pos) {
		case pos_t::LACK_OF_SPACE:
			while (subthreads.size() > idx + 1) {
				subthreads.pop_back();
			}
			return;

		case pos_t::LEFT_BEHIND:
			son = self;
			MUST_TAIL return restore_property(target, idx);

		case pos_t::AFTER_HIGH:
			subthreads.insert(subthreads.begin() + idx + 1, self);
			MUST_TAIL return restore_property(target, idx);

		case pos_t::EXACTLY_IN:
			MUST_TAIL return restore_property(target, idx + 1);

		case pos_t::BEFORE_LOW:
			son.execute();
			MUST_TAIL return restore_property(target, idx);
		}
	}

  public:
	count_t get_time() const { return last_thread().get_time(); }

	void execute_frwrd(count_diff step = 1) {
		assert_property();

		count_t target = get_time() + step;
		debug_thread last{0};

		if (subthreads.size() != 1) {
			last = last_thread();
			remove_last();
		}
		
		restore_property(target);

		while (target - get_time() >= LAST_CHUNK) {
			debug_thread new_last{get_time()};
			debug_thread& old_last = last_thread();

			while (new_last.in_interval(old_last, target) == pos_t::BEFORE_LOW) {
				new_last.execute();
			}
			add_thread(new_last);
		}

		if (get_time() > last.get_time()) {
			last = last_thread();
		}
		add_thread(last);

		while (get_time() < target) {
			last_thread().execute();
		}
	}

	void execute_backwrd(count_diff step = 1) {
		assert_property();

		if (step >= get_time()) {
			while (subthreads.size() != 1) {
				remove_last();
			}

			return;
		}

		count_t target = get_time() - step;
		while (get_time() > target) {
			remove_last();
		}

		count_t curr_time = get_time();
		restore_property(curr_time);
		execute_frwrd(target - curr_time);
	}

	void print_detail() const {
		for (int i = 0; i < subthreads.size(); i++) {
			std::cout << subthreads[i].get_time() << " ";
		}
		std::cout << "\n";
	}
};

int main() {
	anachro_thread test{};

	for (int i = 0; i < (1 << 20); i++) {
		test.execute_frwrd();
		std::cout << test.get_time() << ": ";
		test.print_detail();
		std::cout << "\n";
	}
}
