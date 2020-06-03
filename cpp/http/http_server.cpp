#include "http_server.h"
#include "http_session.h"

template <bool ssl>
HttpServerImpl<ssl>::HttpServerImpl(net::io_context &ctx, const std::string &host, uint16_t port)
    : _host(host)
    , _port(port)
    , _handler_map_ptr(std::make_shared<HandlerMap>())
    , _ctx(ctx)
    , _acceptor(_ctx, {boost::asio::ip::make_address(_host), _port})
    , _socket(_ctx)
{
}


template <bool ssl>
HttpServerImpl<ssl>::HttpServerImpl(net::io_context &ctx, uint16_t port)
    : HttpServerImpl(ctx, "0.0.0.0", port)
{}

template <bool ssl>
HttpServerImpl<ssl>::HttpServerImpl(const std::string &host, uint16_t port)
    : _host(host)
    , _port(port)
    , _handler_map_ptr(std::make_shared<HandlerMap>())
    , _ctx(_inner_ctx)
    , _acceptor(_ctx, {boost::asio::ip::make_address(_host), _port})
    , _socket(_ctx)
{
}


template <bool ssl>
HttpServerImpl<ssl>::HttpServerImpl(uint16_t port)
    : HttpServerImpl("0.0.0.0", port)
{}

template <bool ssl>
HttpServerImpl<ssl>::~HttpServerImpl()
{
}

template <bool ssl>
void HttpServerImpl<ssl>::get(beast::string_view path, const Http::RequestHandler &cb)
{
    _handler_map_ptr->emplace(Http::RouteInfo{path, http::verb::get}, cb);
}

template <bool ssl>
void HttpServerImpl<ssl>::post(beast::string_view path, const Http::RequestHandler &cb)
{
    _handler_map_ptr->emplace(Http::RouteInfo{path, http::verb::post}, cb);
}

template <bool ssl>
void HttpServerImpl<ssl>::run()
{
    do_accept();
    _ctx.run();
}

template <bool ssl>
void HttpServerImpl<ssl>::listen()
{
}

template <bool ssl>
void HttpServerImpl<ssl>::do_accept()
{
    _acceptor.async_accept(_socket, [this](const boost::beast::error_code &ec){

        if (!ec) {
            // SPDLOG_INFO("new connection [{}]", _socket.remote_endpoint());
            auto session_ptr = std::make_shared<HttpSessionImpl<ssl, false>>(_ctx, std::move(_socket));
            session_ptr->register_handler(_handler_map_ptr);
            session_ptr->async_serve();
        }
        do_accept();
    }
    );
}

template class HttpServerImpl<true>;
template class HttpServerImpl<false>;
