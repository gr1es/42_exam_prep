#include "bigint.hpp"
#include <iostream>
#include <sstream>

static int g_pass = 0;
static int g_fail = 0;

static std::string str(const bigint &b)
{
	std::ostringstream oss;
	oss << b;
	return oss.str();
}

static void check(const std::string &label, const std::string &got, const std::string &expected)
{
	if (got == expected)
	{
		g_pass++;
		std::cout << "[PASS] " << label << " -> " << got << std::endl;
	}
	else
	{
		g_fail++;
		std::cout << "[FAIL] " << label << " -> got \"" << got
				   << "\" expected \"" << expected << "\"" << std::endl;
	}
}

static void check_bool(const std::string &label, bool got, bool expected)
{
	check(label, got ? "true" : "false", expected ? "true" : "false");
}

int main()
{
	// construction & default value
	check("default ctor", str(bigint()), "0");
	check("ctor(0)", str(bigint(0)), "0");
	check("ctor(9)", str(bigint(9)), "9");
	check("ctor(1337)", str(bigint(1337)), "1337");

	// copy ctor / assignment independence
	bigint x(123);
	bigint y(x);
	check("copy ctor value", str(y), "123");
	++x;
	check("copy ctor independence (x changed)", str(x), "124");
	check("copy ctor independence (y untouched)", str(y), "123");

	bigint z;
	z = x;
	check("assignment value", str(z), "124");
	++x;
	check("assignment independence (x changed)", str(x), "125");
	check("assignment independence (z untouched)", str(z), "124");

	// addition - no carry
	check("123 + 456", str(bigint(123) + bigint(456)), "579");
	// addition - single carry
	check("9 + 1", str(bigint(9) + bigint(1)), "10");
	// addition - carry chain through multiple digits
	check("999 + 1", str(bigint(999) + bigint(1)), "1000");
	check("99999 + 1", str(bigint(99999) + bigint(1)), "100000");
	// addition - mismatched lengths
	check("5 + 995", str(bigint(5) + bigint(995)), "1000");
	check("100 + 0", str(bigint(100) + bigint(0)), "100");
	check("0 + 0", str(bigint(0) + bigint(0)), "0");
	// operator+= mutates and returns self
	bigint c(10);
	c += bigint(5);
	check("10 += 5", str(c), "15");

	// digitshift left
	check("42 << 0", str(bigint(42) << 0), "42");
	check("42 << 3", str(bigint(42) << 3), "42000");
	check("0 << 5", str(bigint(0) << 5), "0");
	// digitshift right
	check("1337 >> 2", str(bigint(1337) >> 2), "13");
	check("1337 >> 0", str(bigint(1337) >> 0), "1337");
	check("1337 >> 4", str(bigint(1337) >> 4), "0");   // shift >= length
	check("1337 >> 10", str(bigint(1337) >> 10), "0"); // shift >> length
	// compound shift assignment
	bigint d(1337);
	d <<= 4;
	check("d=1337 <<= 4", str(d), "13370000");
	d >>= bigint(2);
	check("d <<=4 then >>= 2", str(d), "133700");

	// prefix / postfix increment
	bigint e(8);
	bigint pre = ++e;
	check("prefix ++8 (returned)", str(pre), "9");
	check("prefix ++8 (e after)", str(e), "9");
	bigint post = e++;
	check("postfix 9++ (returned old)", str(post), "9");
	check("postfix 9++ (e after)", str(e), "10");
	bigint nine9(99);
	++nine9;
	check("prefix carry chain 99 -> ++", str(nine9), "100");

	// comparisons
	bigint a42(42), b100(100);
	check_bool("42 < 100", a42 < b100, true);
	check_bool("100 < 42", b100 < a42, false);
	check_bool("42 <= 42", a42 <= bigint(42), true);
	check_bool("42 > 100", a42 > b100, false);
	check_bool("100 >= 100", b100 >= bigint(100), true);
	check_bool("42 == 42", a42 == bigint(42), true);
	check_bool("42 == 100", a42 == b100, false);
	check_bool("42 != 100", a42 != b100, true);
	check_bool("0 == 0", bigint(0) == bigint(), true);
	check_bool("9 < 10 (length differs)", bigint(9) < bigint(10), true);
	check_bool("100 < 99 (length differs)", bigint(100) < bigint(99), false);

	// zero edge cases on shift operators
	check("0 << 5", str(bigint(0) << 5), "0");
	check("0 >> 5", str(bigint(0) >> 5), "0");
	bigint zero_shift_left(0);
	zero_shift_left <<= bigint(5);
	check("0 <<= 5", str(zero_shift_left), "0");
	bigint zero_shift_right(0);
	zero_shift_right >>= bigint(5);
	check("0 >>= 5", str(zero_shift_right), "0");
	check("5 << 0", str(bigint(5) << 0), "5");
	check("5 >> 0", str(bigint(5) >> 0), "5");
	bigint zero_zero(0);
	zero_zero <<= bigint(0);
	check("0 <<= 0", str(zero_zero), "0");
	zero_zero >>= bigint(0);
	check("0 >>= 0 (chained)", str(zero_zero), "0");

	// shift amount exactly equal to digit count (boundary, not just "beyond")
	check("1337 >> 4 (== length)", str(bigint(1337) >> 4), "0");
	check("100 >> 3 (== length)", str(bigint(100) >> 3), "0");
	check("100 >> 2 (one less than length)", str(bigint(100) >> 2), "1");

	// self-reference / aliasing
	bigint self_add(7);
	self_add += self_add;
	check("self += self (7+7)", str(self_add), "14");

	bigint self_shift_l(2);
	self_shift_l <<= self_shift_l; // shift amount derived from itself (2) -> 2 << 2 -> 200
	check("self <<= self (2 << 2)", str(self_shift_l), "200");

	bigint self_shift_r(23);
	self_shift_r >>= self_shift_r; // shift count = vectorToInt({3,2}) = 23 >= 2 digits -> 0
	check("self >>= self (23 >> 23)", str(self_shift_r), "0");

	bigint self_assign(99);
	self_assign = self_assign;
	check("self assignment (a = a)", str(self_assign), "99");

	// equality between differently-constructed zeros
	check_bool("default-ctor zero == ctor(0)", bigint() == bigint(0), true);
	check_bool("zero from subtraction-like shift == ctor(0)", (bigint(7) >> 1) == bigint(0), true);

	// large carry chain across many digits
	check("9999999 + 1", str(bigint(9999999) + bigint(1)), "10000000");

	// increment crossing multiple 9s
	bigint many_nines(9999);
	++many_nines;
	check("9999 -> ++", str(many_nines), "10000");

	std::cout << "----" << std::endl;
	std::cout << g_pass << " passed, " << g_fail << " failed" << std::endl;
	return (g_fail != 0);
}
