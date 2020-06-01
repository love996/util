#include <initializer_list>
#include <iostream>
#include <type_traits>

#include <boost/beast/ssl.hpp>
#include <boost/beast.hpp>

#include "common/template.h"
#include "http_common.h"
#include "util/output.h"

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
    void async_##method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        async_request_proxy(target, args...); \
    } \


#define HTTP_METHOD(method) \
    SYNC_HTTP_METHOD(method) \
    ASYNC_HTTP_METHOD(method)


template <bool ssl, bool Client>
class HttpSessionImpl: public std::enable_shared_from_this<HttpSessionImpl<ssl, Client>>
{
public:
    HttpSessionImpl(const std::string &host, uint16_t port);
    HttpSessionImpl(net::io_context &ctx, const std::string &host, uint16_t port);
    ~HttpSessionImpl();

    HTTP_METHOD(get)
    HTTP_METHOD(post)

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
    net::io_context _ctx_inner;
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
    
private:
    void reconnect();
    void connect();
    void disconnect();
    void make_request() ;
    Http::Response request();
    void async_request();

    void setParam(const Http::UrlParam &url_param);
    void setParam(const Http::HeadParam &url_param);
    void setParam(const Http::StringBody &body);
    void setParam(const ReadHandler &handler);

    template <typename Stream>
    void do_async(Stream &stream)
    {
        connect();
        auto self(this->shared_from_this());
        beast::http::async_write(stream, _req, [self, this, &stream](beast::error_code const &ec, size_t s)
            {
                if (ec) {
                    CERR << ec.message();
                    return;
                }
                if constexpr(ssl) {
                    COUT << "write bytes:" << s << "read https";
                }
                else {
                    COUT << "write bytes:" << s << "read http";
                }
                beast::http::async_read(stream, _buffer, _resp, [self, this](beast::error_code const &ec, size_t s)
                    {
                        if (ec) {
                            CERR << ec.message();
                            return;
                        }
                        COUT << "read byte:" << s;
                        self->_read_handler(ec, _resp);
                    }
                );
            });
    }
};

// http
using HttpSession = HttpSessionImpl<false, false>;
using HttpClient = HttpSessionImpl<false, true>;

// https
using HttpsSession = HttpSessionImpl<true, false>;
using HttpsClient = HttpSessionImpl<true, true>;
