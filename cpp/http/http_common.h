#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include "common/exception.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;

class HttpException : public virtual Exception
{
};


namespace Http
{
    // 转义一些http请求中的特殊字符
    void convert(std::string &v);
    using Request = boost::beast::http::request<boost::beast::http::string_body>;
    using Response = boost::beast::http::response<boost::beast::http::string_body>;
    using RequestHandler = std::function<void (beast::error_code const&, const Request &)>;
    using ResponseHandler = std::function<void (beast::error_code const&, const Response &)>;

    struct Param : public std::map<std::string, std::string>
    {
    public:
        Param &operator()(const std::string &k, const char *v);
    
        Param &operator()(const std::string &k, std::string &&v);
    
        Param &operator()(const std::string &k, const std::string &v);
    
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
