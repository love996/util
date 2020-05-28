#include "http_common.h"

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
    
        Param &operator()(const std::string &k, const std::string &v)
        {
            (*this)[k] = v;
            return *this;
        }
    
        template <typename Value>
        Param &operator()(const std::string &k, const Value &v)
        {
            (*this)[k] = std::to_string(v);
            return *this;
        }
    
        template <typename Value>
        Param &operator()(const std::string &k, Value &&v)
        {
            (*this)[k] = std::to_string(v);
            return *this;
        }
    };
    struct UrlParam : public Param
    {};
    struct HeadParam : public Param
    {};
    using StringBody = std::string;
}
