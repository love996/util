// #include "http_session.h"
#include <string>
#include "util/output.h"
#include "util/defer.h"


using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::HttpSessionImpl(const std::string &host, uint16_t port)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{_inner_ctx}
    , _resolver(_ctx)
    , _stream(_ctx)
    , _keep_alive(true)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::HttpSessionImpl(net::io_context &ctx, const std::string &host, uint16_t port)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{ctx}
    , _resolver(_ctx)
    , _stream(_ctx)
    , _keep_alive(true)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::HttpSessionImpl(tcp::socket &&socket)
    : _version{11}
    , _host(socket.local_endpoint().address().to_string())
    , _port(socket.local_endpoint().port())
    , _ctx{_inner_ctx}
    , _resolver(_ctx)
    , _stream(std::move(socket))
    , _keep_alive(true)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::HttpSessionImpl(net::io_context &ctx, tcp::socket &&socket)
    : _version{11}
    , _host(socket.local_endpoint().address().to_string())
    , _port(socket.local_endpoint().port())
    , _ctx{ctx}
    , _resolver(_ctx)
    , _stream(std::move(socket))
    , _keep_alive(true)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::register_handler(HandlerMapPtr ptr)
{
    _handler_map_ptr = ptr;
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::reconnect()
{
    disconnect();
    connect();
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::connect()
{
    if constexpr(!Client) {
        assert(false && "server should not call connect");
    }
    else if constexpr(ssl) {
        if (_connected) return;
        auto const results = _resolver.resolve(_host, std::to_string(_port));
        _ssl_ctx.set_verify_mode(ssl::verify_none);
        if(! SSL_set_tlsext_host_name(_ssl_stream.native_handle(), _host.c_str())) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            BOOST_THROW_EXCEPTION(HttpException() << ErrorMsg(ec.message()));
        }
        beast::get_lowest_layer(_ssl_stream).connect(results);
        _ssl_stream.handshake(ssl::stream_base::client);
        _connected = true;
    }
    else {
        if (_connected) return;
        auto const results = _resolver.resolve(_host, std::to_string(_port));
        _stream.connect(results);
    }
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::async_serve()
{
    response();
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::disconnect()
{
    if (!_connected) return ;
    _connected = false;
    return;

    /*
    boost::beast::error_code ec;
    if (_enable_ssl) {
        COUT << "enable ssl";
        _ssl_stream.shutdown(ec);
    }
    else {
        _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    }
    _connected = false;
    if (ec == net::error::eof) {
        CERR << ec.message();
        ec = {};
    }
    if (ec) {
        CERR << ec.message();
        throw beast::system_error{ec};
        // BOOST_THROW_EXCEPTION(ec);
    }
    */
}

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::~HttpSessionImpl()
{
    COUT << "disconnect";
    // if (&_ctx == &_inner_ctx) {
    //     _ctx.run();
    // }
    // disconnect();
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::setParam(const Http::UrlParam &url_param)
{
    _url_param = url_param;
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::setParam(const Http::StringBody &body)
{
    _body = body;
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::make_request()
{
    _req.set(http::field::host, _host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION);
    _req.version(_version);
    _req.keep_alive(_keep_alive);
    // head
    for (auto &[k, v] : _head_param) {
        _req.insert(k, v);
    }

    // url_param
    std::string param;
    for (auto &[k, v] : _url_param) {
        param += "&";
        param += k;
        param += "=";
        Http::convert(v);
        param += v;
    }
    _req.target(std::string(_req.target()) + param);
    if (param.size() > 0) {
        param[0] = '?';
    }

    if (_body.size()) {
        _req.set(http::field::body, _body);
        _req.prepare_payload();
    }
}

template <bool ssl, bool Client>
template <typename Body>
void HttpSessionImpl<ssl, Client>::request(http::response<Body> &resp)
{
    assert(!Client && "server should not call request");
    // static_assert(!Client,  "server should not call request");
    _buffer.clear();
    int retry = 0;
    if (_req.keep_alive()) {
        retry = 1;
    }
    do {
        try {
            connect();
            if constexpr(ssl) {
                http::write(_ssl_stream, _req);
                http::read(_ssl_stream, _buffer, resp);
            }
            else {
                http::write(_stream, _req);
                http::read(_stream, _buffer, resp);
            }
            break;
        }
        catch (std::exception &e) {
            CERR << e.what();
            if (--retry >= 0) {
                reconnect();
            }
            else {
                BOOST_THROW_EXCEPTION(e);
                // throw;
            }
        }
    } while(retry >= 0);
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::do_response(const beast::error_code &ec, size_t s)
{
    if (ec) {
        CERR << ec.message();
        return;
    }
    COUT << "received len:" << s;
    Http::Response resp;
    Defer f([this]
        {
            if constexpr(ssl) {
                http::write(_ssl_stream, resp);
            }
            else {
                http::write(_stream, resp);
            }
        });
    Http::RouteInfo info{_req.target(), _req.method()};
    auto iter = _handler_map_ptr->find(info);
    if (iter == _handler_map_ptr->end()) {
        CERR << _req.target() << ":" << _req.method();
        _resp.result(http::status::not_found);
        return;
    }
    _resp = iter->second(ec, _req);
    _resp.result(http::status::ok);
    response();
}


template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::response()
{
    auto self(this->shared_from_this());
    if constexpr(ssl) {
        http::async_read(_ssl_stream, _buffer, _req, beast::bind_front_handler(&HttpSessionImpl::do_response, self));
    }
    else {
        http::async_read(_stream, _buffer, _req, beast::bind_front_handler(&HttpSessionImpl::do_response, self));
    }
}

template <bool ssl, bool Client>
template <typename Handler>
void HttpSessionImpl<ssl, Client>::async_request(StreamType &stream, const Handler &handler)
{
    if (!Client) {
    }
    else if constexpr(Client) {
        // _resp = Http::Response{};
        _buffer.clear();
        // _resp.clear();
        int retry = 0;
        if (_req.keep_alive()) {
            retry = 1;
        }
        do {
            try {
                do_async_request(stream, handler);
                break;
            }
            catch (std::exception &e) {
                CERR << e.what();
                if (--retry >= 0) {
                    reconnect();
                }
                else {
                    BOOST_THROW_EXCEPTION(e);
                    // throw;
                }
            }
        } while(retry >= 0);
    }
}

template <bool ssl, bool Client>
template <typename Body>
void HttpSessionImpl<ssl, Client>::do_async_request(StreamType &stream, const Http::ResponesHandler<Body> &handler)
{
    connect();
    auto self(this->shared_from_this());
    COUT << _req.method() << ":" << http::verb::get << _req.target();
    beast::http::async_write(stream, _req, [handler, self, this, &stream](beast::error_code const &ec, size_t s)
        {   
            if (ec) {
                CERR << ec.message();
                return;
            }   
            COUT << "write byte:" << s;
            auto resp_ptr = std::make_shared<http::response<Body>>();
            auto &resp = std::get<http::response<Body>&>(std::tuple(_file_resp, _string_resp));
            beast::http::async_read(stream, _buffer, resp, [handler, self, &resp, this](beast::error_code const &ec, size_t s)
                {   
                    if (ec) {
                        CERR << ec.message();
                        return;
                    }   
                    COUT << "read byte:" << s;
                    COUT << "buffer:" << (const char*)_buffer.data().data();
                    handler(ec, resp);
                    // self->_read_handler(ec, resp_ptr);
                }   
            );  
        }); 
}

// template<> void HttpSessionImpl<true, true>::do_async<beast::ssl_stream<beast::tcp_stream>>(beast::ssl_stream<beast::tcp_stream> &);

// client
// template class HttpSessionImpl<true, true>;
// template class HttpSessionImpl<false, true>;
// 
// // server
// template class HttpSessionImpl<true, false>;
// template class HttpSessionImpl<false, false>;

