#include "http_session.h"
#include <boost/asio.hpp>
#include <string>
#include "util/output.h"


using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

template <bool Client>
HttpSessionImpl<Client>::HttpSessionImpl(const std::string &host, uint16_t port, bool enable_ssl)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{}
    , _resolver(_ctx)
    , _stream(_ctx)
    , _keep_alive(true)
    , _enable_ssl(enable_ssl)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}


template <bool Client>
void HttpSessionImpl<Client>::reconnect()
{
    disconnect();
    connect();
}

template <bool Client>
void HttpSessionImpl<Client>::connect()
{
    if (_connected) return;
    auto const results = _resolver.resolve(_host, std::to_string(_port));
    if (_enable_ssl) {
        _ssl_ctx.set_verify_mode(ssl::verify_none);
        if(! SSL_set_tlsext_host_name(_ssl_stream.native_handle(), _host.c_str())) {   
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            BOOST_THROW_EXCEPTION(HttpException() << ErrorMsg(ec.message()));
        }
        beast::get_lowest_layer(_ssl_stream).connect(results);
        _ssl_stream.handshake(ssl::stream_base::client);
    }
    else {
        _stream.connect(results);
    }
    _connected = true;
}

template <bool Client>
void HttpSessionImpl<Client>::disconnect()
{
    if (!_connected) return ;
    _connected = false;
    return;

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
}

template <bool Client>
HttpSessionImpl<Client>::~HttpSessionImpl()
{
    // disconnect();
}

template <bool Client>
void HttpSessionImpl<Client>::setParam(const Http::UrlParam &url_param)
{
    _url_param = url_param;
}

template <bool Client>
void HttpSessionImpl<Client>::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

template <bool Client>
void HttpSessionImpl<Client>::setParam(const Http::StringBody &body)
{
    _body = body;
}

template <bool Client>
void HttpSessionImpl<Client>::setParam(const Http::RequestHandler &handler)
{
    if constexpr(Client) {
        _write_handler = handler;
    }
    else {
        _read_handler = handler;
    }
}

template <bool Client>
void HttpSessionImpl<Client>::setParam(const Http::ResponseHandler &handler)
{
    if constexpr(Client) {
        _read_handler = handler;
    }
    else {
        _write_handler = handler;
    }
}

template <bool Client>
void HttpSessionImpl<Client>::make_request()
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

template <bool Client>
Http::Response HttpSessionImpl<Client>::request()
{
    Http::Response resp;
    int retry = 0;
    if (_req.keep_alive()) {
        retry = 1;
    }
    do {
        try {
            connect();
            if (_enable_ssl) {
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
    return resp;
}

template class HttpSessionImpl<true>;
template class HttpSessionImpl<false>;
