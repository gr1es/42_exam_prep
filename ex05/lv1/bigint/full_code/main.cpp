#include "bigint.hpp"
#include <iostream>

int main()
{
	const bigint a(42);
	bigint b(21), c, d(1337), e(d);

	std::cout << "a = " << a << std::endl;
	std::cout << "b = " << b << std::endl;
	std::cout << "c = " << c << std::endl;
	std::cout << "d = " << d << std::endl;
	std::cout << "e = " << e << std::endl;

	std::cout << "a + b = " << a + b << std::endl;
	std::cout << "(c += a) = " << (c += a) << std::endl;

	// own test (too see if c has been changed)
	std::cout << "c = " << c << std::endl;
	// /own test

	std::cout << "b = " << b << std::endl;
	std::cout << "++b = " << ++b << std::endl;
	std::cout << "b++ = " << b++ << std::endl;

	std::cout << "(b << 10) + 42 = " << ((b << 10) + 42) << std::endl;
	std::cout << "(d <<= 4) = " << (d <<= 4) << std::endl;
	std::cout << "(d >>= 2) = " << (d >>= (const bigint)2) << std::endl;

	// own test (too see if c has been changed)
	std::cout << "d = " << d << std::endl;
	// /own test

	std::cout << "a = " << a << std::endl;
	std::cout << "d = " << d << std::endl;

	std::cout << "(d < a) = " << (d < a) << std::endl;
	std::cout << "(d <= a) = " << (d <= a) << std::endl;
	std::cout << "(d > a) = " << (d > a) << std::endl;
	std::cout << "(d >= a) = " << (d >= a) << std::endl;
	std::cout << "(d == a) = " << (d == a) << std::endl;
	std::cout << "(d != a) = " << (d != a) << std::endl;

	// own test (too see if c has been changed)
	std::cout << "a = " << a << std::endl;
	std::cout << "c = " << c << std::endl;
	std::cout << "(c < a) = " << (c < a) << std::endl;
	std::cout << "(c <= a) = " << (c <= a) << std::endl;
	std::cout << "(c > a) = " << (c > a) << std::endl;
	std::cout << "(c >= a) = " << (c >= a) << std::endl;
	std::cout << "(c == a) = " << (c == a) << std::endl;
	std::cout << "(c != a) = " << (c != a) << std::endl;
	// /own test
	return (0);
}
