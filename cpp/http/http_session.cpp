#include "http_session.h"
#include <boost/asio.hpp>
#include <string>
#include "util/output.h"

/*
class HttpException : public std::exception
{
public:
    HttpException(const std::string &msg)
        : _msg(msg)
    {
    }
    virtual const char *what() const noexcept override
    {
        return _msg.c_str();
    }
private:
    std::string _msg;
};
*/



using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

// std::ostream &operator<<(std::ostream &os, boost::asio::ip::tcp::endpoint ep)
// {
//     return os << ep.address().to_string() << ":" << ep.port();
// }

HttpSession::HttpSession(const std::string &host, uint16_t port, bool ssl)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{}
    , _resolver(_ctx)
    , _stream(_ctx)
    , _keep_alive(true)
    , _enable_ssl(ssl)
    , _connected(false)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
{
}

void HttpSession::reconnect()
{
    disconnect();
    connect();
}

void HttpSession::connect()
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

void HttpSession::disconnect()
{
    if (!_connected) return ;
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

HttpSession::~HttpSession()
{
    // disconnect();
}

void HttpSession::setParam(const Http::UrlParam &url_param)
{
    _url_param = url_param;
}

void HttpSession::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

void HttpSession::setParam(const Http::StringBody &body)
{
    _body = body;
}

void HttpSession::make_request()
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

Http::Response HttpSession::request()
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
