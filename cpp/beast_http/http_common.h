#pragma once
#include <variant>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include "common/exception.h"
#include "common/net.h"

class HttpException : public virtual Exception
{
};


namespace Http
{
    // 转义一些http请求中的特殊字符
    void convert(std::string &v);
    using StringRequest = http::request<http::string_body>;
    // using StringRequest = http::request<http::string_body>;
    using StringResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;
    using Response = std::variant<StringResponse, FileResponse>;


    using ResponseHandler = std::function<void (beast::error_code const&, const Response&)>;
    using StringResponseHandler = ResponseHandler<http::string_body>;
    using FileResponseHandler = ResponseHandler<http::file_body>;

    using Request = std::variant<StringRequest>;

    using RequestHandler = std::function<Response (beast::error_code const&, Request &)>;

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

    struct RouteInfo
    {   
        beast::string_view path;
        http::verb method;
        bool operator<(const RouteInfo &o) const
        {   
            return (this->path < o.path) || (this->path == o.path && this->method < o.method);
        }   
    };
}
