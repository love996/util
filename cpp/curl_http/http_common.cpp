#include "http_common.h"
// #include <boost/algorithm/string.hpp>

namespace Http
{
    Param &Param::operator()(const std::string &k, const char *v)
    {
        (*this)[k] = std::string(v);
        return *this;
    }
    
    Param &Param::operator()(const std::string &k, std::string &&v)
    {
        (*this)[k] = v;
        return *this;
    }
    
    Param &Param::operator()(const std::string &k, const std::string &v)
    {
        (*this)[k] = v;
        return *this;
    }
}
