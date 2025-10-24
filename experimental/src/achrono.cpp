#include "defer.h"
#include "inline.h"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <stdint.h>
#include <sys/types.h>
#include <utility>
#include <variant>
#include <vector>

//		here is next
// |----[_______)-----------|
// 0	^LOW	^HIGH		1

using count_t = uint64_t;
using count_diff = int64_t;
constexpr double LOW_RATIO = 0.5;
constexpr double HIGH_RATIO = 0.75;
constexpr count_t LAST_CHUNK = 100;

static_assert(0 < LOW_RATIO && LOW_RATIO < 1.0, "LOW_RATIO must be in (0, 1)");
static_assert(0 < HIGH_RATIO && HIGH_RATIO < 1.0, "HIGH_RATIO must be in (0, 1)");
static_assert(
	LOW_RATIO < HIGH_RATIO && 
	HIGH_RATIO <= LOW_RATIO + LOW_RATIO * (1.0 - LOW_RATIO), 
	"There is a LOW_RATIO-depentant range in which HIGH_RATIO should be"
);

class debug_thread {
  private:
	count_t count = 0; // number of executed instructions
	/**
	memory, functions, and others data ...
	*/

  public:
	debug_thread(count_t time) : count(time) {}
	count_t get_time() const { return count; }

	void execute(count_diff step = 1 /* maybe some arguments ... */) {
		/**
		... some other fancy actions
		*/
		assert(step >= 0);
		count += step;
	}
};

class anachro_thread {
  private:
	std::vector<debug_thread> subthreads = {debug_thread{0}};

	enum class pos_t {
		AFTER_HIGH,
		BEFORE_LOW,
		EXACTLY_IN,
		LACK_OF_SPACE,
		LEFT_BEHIND,
		UNNECESSARY,
	};

	pos_t in_interval(size_t prev, size_t self, count_t end) const {
		assert(prev < subthreads.size() && self < subthreads.size());

		count_t offset = subthreads[prev].get_time();
		count_diff relative_end = (end - offset);

		if (relative_end < LAST_CHUNK) {
			return pos_t::LACK_OF_SPACE;
		}
		
		count_t count = subthreads[self].get_time();
		
		if (count < offset) {
			return pos_t::LEFT_BEHIND;
		}

		count_t high_bound = offset + relative_end * HIGH_RATIO;
		count_t low_bound = offset + relative_end * LOW_RATIO;

		if (count < low_bound) {
			return pos_t::BEFORE_LOW;
		}

		if (count < high_bound) {
			return pos_t::EXACTLY_IN;
		}

		if (self + 1 < subthreads.size() && subthreads[self + 1].get_time() < high_bound) {
			return pos_t::UNNECESSARY;
		}
		
		return pos_t::AFTER_HIGH;
	}

	count_diff to_interval(const debug_thread& prev, const debug_thread& self, count_t end) const {
		assert(prev.get_time() <= self.get_time());

		count_t offset = prev.get_time();
		count_diff relative_count = (self.get_time() - offset);
		count_diff relative_target = (end - offset) * ((LOW_RATIO + HIGH_RATIO) / 2);

		return relative_target - relative_count;
	}

	count_diff to_interval(size_t prev, size_t self, count_t end) const {
		return to_interval(subthreads[prev], subthreads[self], end);
	}

	template <std::convertible_to<debug_thread> T> void add_thread(T &&el) {
		subthreads.push_back(std::forward<T>(el));
	}

	decltype(auto) last_thread(this auto &&self) {
		assert(self.subthreads.size() != 0);
		return self.subthreads.back();
	}

	void remove_last() {
		assert(subthreads.size());
		subthreads.pop_back();
	}

	INLINE void assert_always(count_t target_end) const {
		assert(subthreads.size() && subthreads[0].get_time() == 0);
		for (size_t i = 1; i < subthreads.size(); ++i) {
			assert(subthreads[i - 1].get_time() < subthreads[i].get_time());
		}
		assert(target_end >= get_time());
	}

	INLINE void assert_always() const { assert_always(get_time()); }

	INLINE void assert_ratio(count_t target_end) const {
		for (size_t i = 1; i + 1 < subthreads.size(); ++i) {
			assert(in_interval(i - 1, i, target_end) == pos_t::EXACTLY_IN);
		}
	}

	INLINE void assert_ratio() const { assert_ratio(get_time()); }

	INLINE void assert_small_end() const {
		assert(subthreads.size() <= 1 ||
			   get_time() - subthreads[subthreads.size() - 2].get_time() < LAST_CHUNK);
	}

	INLINE void assert_small_end(count_t target_end) const {
		assert(target_end - get_time() < LAST_CHUNK);
	}

	INLINE void assert_full_property() const {
		assert_always();
		assert_ratio();
		assert_small_end();
	}

	void restore_ratio(count_t target_end, size_t idx = 0) {
		idx++;

		while (idx < subthreads.size()) {
			pos_t rel_pos = in_interval(idx - 1, idx, target_end);

			switch (rel_pos) {
			case pos_t::AFTER_HIGH:
				subthreads.insert(subthreads.begin() + idx, subthreads[idx - 1]);
				continue;

			case pos_t::BEFORE_LOW:
				subthreads[idx].execute(to_interval(idx - 1, idx, target_end));
				continue;

			case pos_t::EXACTLY_IN:
				idx++;
				continue;

			case pos_t::LACK_OF_SPACE:
				subthreads.erase(subthreads.begin() + idx + 1, subthreads.end());
				return;

			case pos_t::LEFT_BEHIND:
				subthreads[idx] = subthreads[idx - 1];
				continue;

			case pos_t::UNNECESSARY:
				subthreads.erase(subthreads.begin() + idx);
				continue;
			}
		}
	}

	void restore_tail(count_t target_end) {
		while (target_end - get_time() >= LAST_CHUNK) {
			debug_thread &old_last = last_thread();
			debug_thread new_last{old_last};

			new_last.execute(to_interval(old_last, new_last, target_end));

			add_thread(new_last);
		}

		debug_thread last = last_thread();
		remove_last();

		if (subthreads.size() == 0 || target_end - get_time() >= LAST_CHUNK) {
			add_thread(last);
		}
		add_thread(last);

		last_thread().execute(target_end - get_time());
	}

	void restore_property(count_t target_end) {
		assert_always(target_end);

		restore_ratio(target_end);
		assert_always(target_end);
		assert_ratio(target_end);

		restore_tail(target_end);
		assert(get_time() == target_end);
		assert_full_property();
	}

  public:
	count_t get_time() const { return last_thread().get_time(); }

	void execute_frwrd(count_diff step = 1) {
		assert_full_property();
		assert(step >= 0);
		defer(assert_full_property());

		restore_property(get_time() + step);
	}

	void execute_backwrd(count_diff step = 1) {
		assert_full_property();
		defer(assert_full_property());

		if (step >= get_time()) {
			subthreads.erase(subthreads.begin() + 1, subthreads.end());
			return;
		}

		count_t target_end = get_time() - step;
		while (get_time() > target_end) {
			remove_last();
		}

		count_t curr_time = get_time();
		restore_ratio(target_end);
		restore_tail(target_end);
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

	size_t jmps;
	std::cin >> jmps;
	
	for (int i = 0; i < jmps; i++) {
		static int64_t jmp;
		std::cin >> jmp;

		if (jmp < 0) {
			test.execute_backwrd(-jmp);
		} else {
			test.execute_frwrd(jmp);
		}

		test.print_detail();
	}
}
