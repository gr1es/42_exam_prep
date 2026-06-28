#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <iostream>
#include <vector>

class bigint
{
	private:
		std::vector<int> _n;

	public:
		bigint();
		bigint(unsigned int);
		bigint(const bigint &other);
		bigint &operator=(const bigint &other);
		~bigint();

		// comparison
		bool operator==(const bigint &b) const;
		bool operator!=(const bigint &b) const;
		bool operator<(const bigint &b) const;
		bool operator<=(const bigint &b) const;
		bool operator>(const bigint &b) const;
		bool operator>=(const bigint &b) const;
		// addition
		bigint operator+(const bigint &b) const;
		bigint &operator+=(const bigint &b);
		//digitshift
		bigint operator<<(const bigint &b) const;
		bigint &operator<<=(const bigint &b);
		bigint operator>>(const bigint &b) const;
		bigint &operator>>=(const bigint &b);
		//prefix
		bigint &operator++();
		bigint operator++(int n);
};

std::ostream &operator<<(std::ostream &os, const bigint &b);

#endif
