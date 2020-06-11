#pragma once
#include "http_common.h"

#define SYNC_HTTP_METHOD(method) \
    template <typename ReqBody, typename RespBody> \
    void async_##method(beast::string_view path, const Http::RequestHandler<ReqBody, RespBody> &handler) \
    { \
    }

#define HTTP_METHOD(method) \
    ASYNC_HTTP_METHOD(method)

template <bool ssl>
class HttpServerImpl
{
public:
    HttpServerImpl(const std::string &host, uint16_t port);
    HttpServerImpl(uint16_t port);

    HttpServerImpl(net::io_context &ctx, const std::string &host, uint16_t port);
    HttpServerImpl(net::io_context &ctx, uint16_t port);

    template <typename ReqBody, typename RespBody>
    void get(beast::string_view path, const Http::RequestHandler<ReqBody, RespBody> &handler)
    {
    }
    void run();
    ~HttpServerImpl();
private:
    std::string _host;
    uint16_t _port;
    std::shared_ptr<std::map<Http::RouteInfo, Http::RequestHandler>> _handler_map_ptr;
    net::io_context _inner_ctx;
    net::io_context &_ctx;
    tcp::acceptor _acceptor;
    tcp::socket _socket;
private:
    void listen();
    void do_accept();
};

using HttpServer = HttpServerImpl<false>;
using HttpsServer = HttpServerImpl<true>;
