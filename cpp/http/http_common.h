#include <boost/beast.hpp>
#include <spdlog/spdlog.h>

namespace Http
{
    using Request = boost::beast::http::request<boost::beast::http::string_body>;
    using Response = boost::beast::http::response<boost::beast::http::string_body>;
    struct Param : public std::map<std::string, std::string>
    {
    public:
        Param &operator()(const std::string &k, const char *v)
        {
            (*this)[k] = v;
            return *this;
        }
    
        Param &operator()(const std::string &k, std::string &&v)
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
