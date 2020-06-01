#pragma once
#include "http_common.h"

template <bool ssl>
class HttpServerImpl
{
public:
    HttpServerImpl(const std::string &host, uint16_t port);
    HttpServerImpl(uint16_t port);
    void get(beast::string_view path, const Http::RequestHandler &handler);
    void post(beast::string_view path, const Http::RequestHandler &handler);
    void run();
    ~HttpServerImpl();
private:
    std::string _host;
    uint16_t _port;
    // Http::RequestHandler _get_handler;
    // Http::RequestHandler _post_handler;
    struct RouteInfo
    {
        beast::string_view path;
        http::verb method;
        bool operator<(const RouteInfo &o)
        {
            return (this->path < o.path) || (this->path == o.path && this->method < o.method);
        }
    };
    std::map<RouteInfo, Http::RequestHandler> _handler_map;
    net::io_context _ctx_inner;
    net::io_context &_ctx;
    net::ip::tcp::acceptor _acceptor;
    net::ip::tcp::socket _socket;
private:
    void listen();
    void do_accept();
};
