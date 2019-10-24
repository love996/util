#pragma once

#include <vector>
#include <iterator>


template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
    return os;
}
