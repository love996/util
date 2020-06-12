#pragma once
#include <variant>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include "common/exception.h"

class HttpException : public virtual Exception
{
};


namespace Http
{
    struct CeilData
    {
        std::string value;
        std::string filename;
    };
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
    struct FormDataParam : public std::map<std::string, CeilData>
    {
    };
    using StringBody = std::string;

    struct StringResponse
    {
        std::vector<HeadParam> header_list;
        std::string body;
    };

    struct FileResponse
    {
        std::vector<HeadParam> header_list;
        std::string filename;
    };
}
