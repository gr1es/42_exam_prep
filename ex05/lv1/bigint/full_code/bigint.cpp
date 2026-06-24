#include "bigint.hpp"

// helper forward declaration
static std::vector<int> intToVector(unsigned int n);
static unsigned int vectorToInt(const std::vector<int> &n);

// OCF
bigint::bigint() : _n({ 0 })
{
}

bigint::bigint(unsigned int n) : _n(intToVector(n))
{
}

bigint::bigint(const bigint &other)
{
	_n = other._n;
}

bigint &bigint::operator=(const bigint &other)
{
	if (this != &other)
		_n = other._n;
	return (*this);
}

bigint::~bigint()
{
}

// COMPARISON
bool bigint::operator==(const bigint &other) const
{
	return (_n == other._n);
}

bool bigint::operator!=(const bigint &other) const
{
	return (_n != other._n);
}

bool bigint::operator<(const bigint &other) const
{
	if (_n.size() != other._n.size())
		return (_n.size() < other._n.size());
	std::vector<int>::const_reverse_iterator rit = _n.crbegin();
	std::vector<int>::const_reverse_iterator rit_o = other._n.crbegin();
	while (rit != _n.crend() && rit_o != other._n.crend())
	{
		if (*rit != *rit_o)
			return (*rit < *rit_o);
		rit++;
		rit_o++;
	}
	return (false);
}

bool bigint::operator<=(const bigint &other) const
{
	if (_n.size() != other._n.size())
		return (_n.size() < other._n.size());
	std::vector<int>::const_reverse_iterator rit = _n.crbegin();
	std::vector<int>::const_reverse_iterator rit_o = other._n.crbegin();
	while (rit != _n.crend() && rit_o != other._n.crend())
	{
		if (*rit != *rit_o)
			return (*rit < *rit_o);
		rit++;
		rit_o++;
	}
	return (true);
}

bool bigint::operator>(const bigint &other) const
{
	if (_n.size() != other._n.size())
		return (_n.size() > other._n.size());
	std::vector<int>::const_reverse_iterator rit = _n.crbegin();
	std::vector<int>::const_reverse_iterator rit_o = other._n.crbegin();
	while (rit != _n.crend() && rit_o != other._n.crend())
	{
		if (*rit != *rit_o)
			return (*rit > *rit_o);
		rit++;
		rit_o++;
	}
	return (false);
}

bool bigint::operator>=(const bigint &other) const
{
	if (_n.size() != other._n.size())
		return (_n.size() > other._n.size());
	std::vector<int>::const_reverse_iterator rit = _n.crbegin();
	std::vector<int>::const_reverse_iterator rit_o = other._n.crbegin();
	while (rit != _n.crend() && rit_o != other._n.crend())
	{
		if (*rit != *rit_o)
			return (*rit > *rit_o);
		rit++;
		rit_o++;
	}
	return (true);
}

// ADDITION
bigint bigint::operator+(const bigint &other) const
{
	std::vector<int> res_n;
	int temp = 0;
	std::vector<int>::const_iterator it = _n.cbegin();
	std::vector<int>::const_iterator it_o = other._n.cbegin();
	while (true)
	{
		int sum = 0;
		if (it == _n.end() && it_o == other._n.end())
			break;
		if (it != _n.end() && it_o != other._n.end())
		{
			sum += *it + *it_o + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it++;
			it_o++;
		}
		else if (it != _n.end())
		{
			sum = *it + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it++;
		}
		else if (it_o != other._n.end())
		{
			sum = *it_o + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it_o++;
		}
	}
	if (temp != 0)
		res_n.push_back(temp);
	bigint res;
	res._n = res_n;
	return (res);
}

bigint &bigint::operator+=(const bigint &other)
{
	std::vector<int> res_n;
	int temp = 0;
	std::vector<int>::const_iterator it = _n.cbegin();
	std::vector<int>::const_iterator it_o = other._n.cbegin();
	while (true)
	{
		int sum = 0;
		if (it == _n.end() && it_o == other._n.end())
			break;
		if (it != _n.end() && it_o != other._n.end())
		{
			sum += *it + *it_o + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it++;
			it_o++;
		}
		else if (it != _n.end())
		{
			sum = *it + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it++;
		}
		else if (it_o != other._n.end())
		{
			sum = *it_o + temp;
			res_n.push_back(sum % 10);
			temp = sum / 10;
			it_o++;
		}
	}
	if (temp != 0)
		res_n.push_back(temp);
	_n = res_n;
	return (*this);
}

// DIGIT SHIFT
bigint bigint::operator<<(const bigint &b) const
{
	std::vector<int> res_n = _n;
	// insert elegantly adds 0 n times to the iterator at the beginning of the vector
	res_n.insert(res_n.begin(), vectorToInt(b._n), 0);
	bigint res;
	res._n = res_n;
	return (res);
}

bigint &bigint::operator<<=(const bigint &b)
{
	_n.insert(_n.begin(), vectorToInt(b._n), 0);
	return (*this);
}

bigint bigint::operator>>(const bigint &b) const
{
	std::vector<int> res_n;
	if (b._n.size() >= _n.size())
		res_n = { 0 };
	else
	{
		std::vector<int>::const_iterator it = _n.cbegin();
		for (size_t i = 0; i < vectorToInt(b._n); i++)
			it++;
		res_n = { it, _n.cend() };
	}
	bigint res;
	res._n = res_n;
	return (res);
}

bigint &bigint::operator>>=(const bigint &b)
{
	std::vector<int> res_n;
	if (b._n.size() >= _n.size())
		res_n = { 0 };
	else
	{
		std::vector<int>::const_iterator it = _n.cbegin();
		for (size_t i = 0; i < vectorToInt(b._n); i++)
			it++;
		res_n = { it, _n.cend() };
	}
	_n = res_n;
	return (*this);
}

// PREFIX / POSTFIX

bigint &bigint::operator++()
{
	*this += 1;
	return (*this);
}

bigint bigint::operator++(int n)
{
	(void)n;
	bigint temp(*this);
	*this += 1;
	return(temp);
}

// STREAM OUTPUT
std::ostream &operator<<(std::ostream &os, const bigint &b)
{
	std::string n = "";
	for (std::vector<int>::const_reverse_iterator rit = b.getN().crbegin(); rit != b.getN().crend(); rit++)
		n += *rit + '0';
	return (os << n);
}

const std::vector<int> &bigint::getN() const
{
	return (_n);
}

// helpers
static std::vector<int> intToVector(unsigned int n)
{
	std::vector<int> res;
	while (n != 0)
	{
		res.push_back(n % 10);
		n /= 10;
	}
	return (res);
}

static unsigned int vectorToInt(const std::vector<int> &n)
{
	unsigned int res = 0;

	for (std::vector<int>::const_reverse_iterator rit = n.rbegin(); rit != n.rend(); rit++)
		res = (res * 10) + *rit;
	return (res);
}
