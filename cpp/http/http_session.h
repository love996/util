#include <initializer_list>
#include <iostream>
#include <type_traits>

#include <boost/beast/ssl.hpp>
#include <boost/beast.hpp>

#include "common/template.h"
#include "http_common.h"
#include "util/output.h"

// using ReadCallback = std::function<void (const beast::error_code &, size_t)>;


// static_assert(contains<std::string, Args...>::value, "不应包含回调函数");
// static_assert(contains<std::string, Args...>::value == false, "必须包含回调函数");

#define SYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    Http::Response method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        return request_proxy(target, args...); \
    }

#define ASYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    void async_##method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        async_request_proxy(target, args...); \
    }


#define HTTP_METHOD(method) \
    SYNC_HTTP_METHOD(method) \
    ASYNC_HTTP_METHOD(method)

using HandlerMap = std::map<Http::RouteInfo, Http::RequestHandler>;
using HandlerMapPtr = std::shared_ptr<std::map<Http::RouteInfo, Http::RequestHandler>>;

template <bool ssl, bool Client>
class HttpSessionImpl: public std::enable_shared_from_this<HttpSessionImpl<ssl, Client>>
{
public:
    HttpSessionImpl(const std::string &host, uint16_t port);
    HttpSessionImpl(net::io_context &ctx, const std::string &host, uint16_t port);
    HttpSessionImpl(net::io_context &ctx, tcp::socket &&socket);
    HttpSessionImpl(tcp::socket &&socket);
    ~HttpSessionImpl();

    // 做server 端
    void register_handler(HandlerMapPtr ptr);

    HTTP_METHOD(get)
    HTTP_METHOD(post)

    void async_serve();
private:
    template <typename ...Args>
    void make_request_proxy(const std::string &target, const Args &...args)
    {
        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        boost::ignore_unused(arr);
        _req.target(target);
        make_request();
    }

    template <typename ...Args>
    Http::Response request_proxy(const std::string &target, const Args &...args)
    {
        make_request_proxy(target, args...);
        return request();
    }

    template <typename ...Args>
    void async_request_proxy(const std::string &target, const Args &...args)
    {
        make_request_proxy(target, args...);
        async_request();
    }


private:
    int _version;
    std::string _host;
    uint16_t _port;
    net::io_context _inner_ctx;
    net::io_context &_ctx;
    net::ip::tcp::resolver _resolver;
    beast::tcp_stream _stream;
    bool _keep_alive;
    bool _connected;

    beast::flat_buffer _buffer;
    Http::UrlParam _url_param;
    Http::StringBody _body;
    Http::HeadParam _head_param;
    Http::Request _req;
    Http::Response _resp;
    // ssl
    net::ssl::context _ssl_ctx;
    beast::ssl_stream<beast::tcp_stream> _ssl_stream;

    // is_client;
    using ReadHandler = std::conditional_t<Client, Http::ResponseHandler, Http::RequestHandler>;
    ReadHandler _read_handler;

    HandlerMapPtr _handler_map_ptr;
    
private:
    void reconnect();
    void connect();
    void disconnect();
    void make_request() ;
    Http::Response request();
    void response();
    void async_request();

    void setParam(const Http::UrlParam &url_param);
    void setParam(const Http::HeadParam &url_param);
    void setParam(const Http::StringBody &body);
    void setParam(const ReadHandler &handler);

    template <typename Stream>
    void do_async_request(Stream &stream);

    void do_response(const beast::error_code &ec, size_t s);
};

// http
using HttpSession = HttpSessionImpl<false, false>;
using HttpClient = HttpSessionImpl<false, true>;

// https
using HttpsSession = HttpSessionImpl<true, false>;
using HttpsClient = HttpSessionImpl<true, true>;
