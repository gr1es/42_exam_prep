#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <string>

class bigint
{
	private:
		// TODO: container of some kind - or string?
	public:
		// OCF
		bigint();
		bigint(unsigned int nbr);
		bigint(const bigint &other);
		bigint &operator=(const bigint &other);
		~bigint();

		// comparsion to another bigint object
		bool operator==(const bigint &other);
		bool operator!=(const bigint &other);
		bool operator<(const bigint &other);
		bool operator<=(const bigint &other);
		bool operator>(const bigint &other);
		bool operator>=(const bigint &other);
		// addition
		bigint &operator+(const bigint &other);
		bigint &operator+=(const bigint &other);
		// digitshift
		bigint &operator<<(unsigned int n);
		bigint &operator<<=(unsigned int n);
		bigint &operator>>(unsigned int n);
		bigint &operator>>=(unsigned int n);
		// prefix, postfix
		bigint &operator++();
		bigint &operator++(int n);
};

std::string &operator<<(std::ostream &os, const bigint &other);

#endif
