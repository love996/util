#include "http_client.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <boost/asio.hpp>
// #include <boost/beast/http/message.hpp>
#include <string>

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

std::ostream &operator<<(std::ostream &os, boost::asio::ip::tcp::endpoint ep)
{
    return os << ep.address().to_string() << ":" << ep.port();
}

HttpClient::HttpClient(const std::string &host, uint16_t port, bool ssl)
    : _version{11}
    , _host(host)
    , _port(port)
    , _ctx{}
    , _resolver(_ctx)
    , _stream(_ctx)
    , _ssl_ctx(ssl::context::tlsv12_client)
    , _ssl_stream(_ctx, _ssl_ctx)
    , _enable_ssl(ssl)
{
    reconnect();
}

void HttpClient::reconnect()
{
    if (_connected) return;
    SPDLOG_INFO("connecting [{}:{}] ...", _host, _port);
    auto const results = _resolver.resolve(_host, std::to_string(_port));
    for (auto &result : results) {
        SPDLOG_INFO("result: [{}]", result.endpoint());
    }
    try {
        if (_enable_ssl) {
            SPDLOG_INFO("https init");
            _ssl_ctx.set_verify_mode(ssl::verify_none);
            if(! SSL_set_tlsext_host_name(_ssl_stream.native_handle(), _host.c_str()))
            {   
                beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
                SPDLOG_ERROR("SSL_set_tlsext_host_name [{}]", ec.message());
            }
            beast::get_lowest_layer(_ssl_stream).connect(results);
            _ssl_stream.handshake(ssl::stream_base::client);
        }
        else {
            SPDLOG_INFO("http init");
            _stream.connect(results);
        }
    }
    catch (beast::error_code &ec) {
        SPDLOG_ERROR("connect error [{}]", ec.message());
        return ;
    }
    catch (boost::system::system_error &e) {
        SPDLOG_ERROR("connect error [{}]", e.what());
        return;
    }
}

void HttpClient::connect(const std::string &host, uint16_t port)
{
    if (_host != host) {
        _connected = false;
        _host = host;
    }
    if (_port != port) {
        _connected = false;
        _port = port;
    }
    reconnect();
}

HttpClient::~HttpClient()
{
    boost::beast::error_code ec;
    _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        SPDLOG_INFO("disconnect [{}] error[{}]", _stream.socket().remote_endpoint(), ec.message());
        return;
    }
    SPDLOG_INFO("disconnect [{}]", _stream.socket().remote_endpoint());
}

void HttpClient::setParam(const Http::UrlParam &url_param)
{
    _url_param = url_param;
}

void HttpClient::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

void HttpClient::setParam(const Http::StringBody &body)
{
    _body = body;
}
