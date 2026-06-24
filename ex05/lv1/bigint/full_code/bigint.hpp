#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <iostream>
#include <string>
#include <vector>

class bigint
{
	private:
		std::vector<int> _n;

	public:
		// OCF
		bigint();
		bigint(unsigned int n);
		bigint(const bigint &other);
		bigint &operator=(const bigint &other);
		~bigint();

		// COMPARISON
		bool operator==(const bigint &other) const;
		bool operator!=(const bigint &other) const;
		bool operator<(const bigint &other) const;
		bool operator<=(const bigint &other) const;
		bool operator>(const bigint &other) const;
		bool operator>=(const bigint &other) const;
		// ADDITION
		// functions take bigint but also work with int literal argument thanks to implicit conversion (constructs via parameterized constructor using the int argument)
		// const argument for both!
		bigint operator+(const bigint &other) const;
		bigint &operator+=(const bigint &other);
		// DIGIT SHIFT
		// = operators change original bigint, hence reference return
		bigint operator<<(const bigint &b) const;
		bigint &operator<<=(const bigint &b);
		bigint operator>>(const bigint &b) const;
		bigint &operator>>=(const bigint &b);
		// PREFIX
		// returns modified old bigint by reference
		bigint &operator++();
		// POSTFIX
		// returns old bigint by value
		bigint operator++(int n);

		// getter - necessary for the ostream << override
		const std::vector<int> &getN() const;
};

std::ostream &operator<<(std::ostream &os, const bigint &b);

#endif
