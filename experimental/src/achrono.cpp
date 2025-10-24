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

/////////// TRICKS (from Duckling and others) /////////////

#if defined(__clang__) && __clang__ >= 13
	#define MUST_TAIL [[clang::musttail]]
#elif defined(__GNUG__) && __GNUG__ >= 15
	#define MUST_TAIL [[gnu::musttail]]
#else
	#define MUST_TAIL
	#warning "Using tailcalls without a support from compiler"
#endif

#if defined(__clang__) && __clang__ >= 15
	#define INLINE [[clang::always_inline]]
#elif defined(__GNUG__) && __GNUG__ >= 6
	#define INLINE [[gnu::always_inline]]
#else
	#define INLINE
	#warning "Using INLINE without a support from compiler"
#endif

namespace detail {
	template<typename ActionT>
	class DeferHelper final {
		ActionT action;

	public:
		DeferHelper(ActionT&& action): action(std::move(action)) {}

		~DeferHelper() noexcept { action(); }
	};
}

#define defer(code) ::detail::DeferHelper _([&]() noexcept -> void { code; })

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

	pos_t in_interval(const debug_thread &prev, count_t target) const {
		count_t offset = prev.get_time();
		count_diff relative_count = (count - offset), relative_target = (target - offset);

		if (relative_target < LAST_CHUNK) {
			return pos_t::LACK_OF_SPACE;
		}

		if (relative_count < 0) {
			return pos_t::LEFT_BEHIND;
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

	template <std::convertible_to<debug_thread> T> void add_thread(T &&el) {
		subthreads.push_back(std::forward<T>(el));
	}

	decltype(auto) last_thread(this auto &&self) { return self.subthreads.back(); }

	void remove_last() { subthreads.pop_back(); }

	INLINE void assert_always() const {
		assert(subthreads.size() && subthreads[0].get_time() == 0);
		for (size_t i = 0; i + 1 < subthreads.size(); ++i) {
			assert(subthreads[i].get_time() < subthreads[i + 1].get_time());
		}
	}

	INLINE void assert_ratio() const {
		for (size_t i = 1; i + 1 < subthreads.size(); ++i) {
			assert(subthreads[i].in_interval(subthreads[i - 1], get_time()) == pos_t::EXACTLY_IN);
		}
	}

	INLINE void assert_small_end() const {
		assert(subthreads.size() <= 1 ||
			   get_time() - subthreads[subthreads.size() - 2].get_time() < LAST_CHUNK);
	}

	INLINE void assert_full_property() const {
		assert_always();
		assert_ratio();
		assert_small_end();
	}

	void restore_ratio(count_t target, size_t idx = 0) {
		assert_always();
		idx++;

		while (idx < subthreads.size()) {
			debug_thread &prev = subthreads[idx - 1];
			debug_thread &self = subthreads[idx];

			pos_t rel_pos = self.in_interval(prev, target);

			switch (rel_pos) {
			case pos_t::LACK_OF_SPACE:
				subthreads.erase(subthreads.begin() + idx, subthreads.end());
				return;

			case pos_t::LEFT_BEHIND:
				self = prev;
				continue;

			case pos_t::AFTER_HIGH:
				subthreads.insert(subthreads.begin() + idx, prev);
				continue;

			case pos_t::EXACTLY_IN:
				idx++;
				continue;

			case pos_t::BEFORE_LOW:
				self.execute();
				continue;
			}
		}
	}

  public:
	count_t get_time() const { return last_thread().get_time(); }

	void execute_frwrd(count_diff step = 1) {
		assert_full_property();

		count_t target = get_time() + step;
		debug_thread last{0};

		if (subthreads.size() != 1) {
			last = last_thread();
			remove_last();
		}

		restore_ratio(target);

		while (target - get_time() >= LAST_CHUNK) {
			debug_thread new_last{get_time()};
			debug_thread &old_last = last_thread();

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
		assert_full_property();

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
		restore_ratio(curr_time);
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

	for (int i = 0; i < (1 << 5); i++) {
		test.execute_frwrd((1 << 20));
		std::cout << test.get_time() << ": ";
		test.print_detail();
		std::cout << "\n";
	}
}
