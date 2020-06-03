#pragma once

#include <vector>
#include <iterator>
#include <sstream>
#include <iostream>


template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
    return os;
}

class CoutHelper
{
public:
    CoutHelper(std::ostream &os)
        :_os(os)
    {
    }
    ~CoutHelper()
    {
        _os << _ioss.str() << std::endl << std::flush;
    }
    template <typename T>
    CoutHelper &operator<<(const T& t) {
        _ioss << t << ' ';
        return *this;
    }
private:
    std::ostream &_os;
    std::stringstream _ioss;
};

#define OUT(obj) \
    obj << "(" << __FILE__ << ":" << __LINE__ << ") "

#define CERR \
    OUT(CoutHelper(std::cerr))

#define COUT \
    OUT(CoutHelper(std::cout))
