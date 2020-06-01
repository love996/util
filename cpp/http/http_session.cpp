#include "http_session.h"
#include <boost/asio.hpp>
#include <string>
#include "util/output.h"


using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

template <bool ssl, bool Client>
HttpSessionImpl<ssl, Client>::HttpSessionImpl(const std::string &host, uint16_t port)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{_ctx_inner}
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
void HttpSessionImpl<ssl, Client>::reconnect()
{
    disconnect();
    connect();
}

template <>
void HttpSessionImpl<true, true>::connect()
{
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

template <>
void HttpSessionImpl<false, true>::connect()
{
    if (_connected) return;
    auto const results = _resolver.resolve(_host, std::to_string(_port));
    _stream.connect(results);
}


template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::connect()
{
    assert(false && "server should not call connect");
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
    if (&_ctx == &_ctx_inner) {
        _ctx.run();
    }
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
void HttpSessionImpl<ssl, Client>::setParam(const ReadHandler &handler)
{
    _read_handler = handler;
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::make_request()
{
    _req.set(http::field::host, _host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION);
    _req.version(_version);
    _req.method(http::verb::post);
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
Http::Response HttpSessionImpl<ssl, Client>::request()
{
    assert(!Client && "server should not call request");
    // static_assert(!Client,  "server should not call request");
    _buffer.clear();
    _resp.clear();
    int retry = 0;
    if (_req.keep_alive()) {
        retry = 1;
    }
    do {
        try {
            connect();
            if constexpr(ssl) {
                http::write(_ssl_stream, _req);
                http::read(_ssl_stream, _buffer, _resp);
            }
            else {
                http::write(_stream, _req);
                http::read(_stream, _buffer, _resp);
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
    return std::move(_resp);
}

template <bool ssl, bool Client>
void HttpSessionImpl<ssl, Client>::async_request()
{
    if constexpr(Client) {
        // _resp = Http::Response{};
        _buffer.clear();
        _resp.clear();
        int retry = 0;
        if (_req.keep_alive()) {
            retry = 1;
        }
        do {
            try {
                if constexpr(ssl) {
                    do_async(_ssl_stream);
                }
                else {
                    do_async(_stream);
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
}
// client
template class HttpSessionImpl<true, true>;
template class HttpSessionImpl<false, true>;

// server
template class HttpSessionImpl<true, false>;
template class HttpSessionImpl<false, false>;
