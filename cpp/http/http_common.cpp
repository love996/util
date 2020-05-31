#include "http_common.h"
#include <boost/algorithm/string.hpp>

namespace Http
{
    Param &Param::operator()(const std::string &k, const char *v)
    {
        return (*this)(k, std::string(v));
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
    
    void convert(std::string &v)
    {
        boost::replace_all(v, "+", "%2B");
        boost::replace_all(v, " ", "%20");
        boost::replace_all(v, "/", "%2F");
        boost::replace_all(v, "?", "%3F");
        boost::replace_all(v, "%", "%25");
        boost::replace_all(v, "#", "%23");
        boost::replace_all(v, "&", "%26");
        boost::replace_all(v, "=", "%3D");
    }
}
