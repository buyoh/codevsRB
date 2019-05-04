#pragma once
#include "Macro.h"

struct XorShift {
	using result_type = uint64_t;
	result_type x_;
	XorShift(result_type x = 88172645463325252ull) :x_(x) {};
	static constexpr inline result_type min() { return 0ull; }
	static constexpr inline result_type max() { return numeric_limits<result_type>::max(); }
	inline result_type operator()() { x_ ^= x_ << 7; return x_ ^= x_ >> 9; }
	inline void discard(unsigned long long z) { while (z--) operator()(); }
};
XorShift randdev;