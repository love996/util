#include <initializer_list>
#include <iostream>
#include <type_traits>

#include <boost/beast/ssl.hpp>
#include <boost/beast.hpp>

#include "common/template.h"
#include "http_common.h"

using ReadCallback = std::function<void (const beast::error_code &, size_t)>;

#define SYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    Http::Response method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        return request_proxy(target, args...); \
    } \



// static_assert(contains<std::string, Args...>::value, "不应包含回调函数"); \
// static_assert(contains<std::string, Args...>::value == false, "必须包含回调函数");

#define ASYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    Http::Response async_##method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        return async_request_proxy(target, args...); \
    } \


#define HTTP_METHOD(method) \
    SYNC_HTTP_METHOD(method) \
    ASYNC_HTTP_METHOD(method)


template <bool Client>
class HttpSessionImpl: public std::enable_shared_from_this<HttpSessionImpl<Client>>
{
public:
    HttpSessionImpl(const std::string &host, uint16_t port, bool enable_ssl);
    ~HttpSessionImpl();

    HTTP_METHOD(get)
    HTTP_METHOD(post)

    template <typename ...Args>
    Http::Response request_proxy(const std::string &target, const Args &...args)
    {
        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        boost::ignore_unused(arr);
        _req.target(target);

        make_request();
        return request();
    }

    template <typename ...Args>
    Http::Response async_request_proxy(const std::string &target, const Args &...args)
    {
        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        boost::ignore_unused(arr);
        _req.target(target);

        make_request();
        return request();
    }


    bool ssl() const
    {
        return _enable_ssl;
    }
private:
    int _version;
    std::string _host;
    uint16_t _port;
    net::io_context _ctx;
    net::ip::tcp::resolver _resolver;
    beast::tcp_stream _stream;
    bool _keep_alive;
    bool _enable_ssl;
    bool _connected;


    beast::flat_buffer _buffer;
    Http::UrlParam _url_param;
    Http::StringBody _body;
    Http::HeadParam _head_param;
    Http::Request _req;
    // ssl
    net::ssl::context _ssl_ctx;
    beast::ssl_stream<beast::tcp_stream> _ssl_stream;

    // is_client;
    std::conditional_t<Client, Http::RequestHandler, Http::ResponseHandler> _write_handler;
    std::conditional_t<Client, Http::ResponseHandler, Http::RequestHandler> _read_handler;
    
private:
    void reconnect();
    void connect();
    void disconnect();
    void make_request() ;
    Http::Response request();

    void setParam(const Http::UrlParam &url_param);
    void setParam(const Http::HeadParam &url_param);
    void setParam(const Http::StringBody &body);
    void setParam(const Http::RequestHandler &cb);
    void setParam(const Http::ResponseHandler &cb);
};

using HttpSession = HttpSessionImpl<false>;
using HttpClient = HttpSessionImpl<true>;
