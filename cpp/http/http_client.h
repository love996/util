#include <string>
#include <initializer_list>
#include <iostream>
#include <spdlog/spdlog.h>
#include <boost/beast/ssl.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>

#include "http_common.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;


class HttpClient : public std::enable_shared_from_this<HttpClient>
{
public:
    // HttpClient(bool ssl);
    HttpClient(const std::string &host, uint16_t port, bool ssl = false);
    void connect(const std::string &host, uint16_t port);
    void reconnect();
    template <typename ...Args>
    Http::Response get(const std::string &target, const Args &...args)
    {
        _req = Http::Request{http::verb::get, target, _version};
        return request(target, args...);
    }
    template <typename ...Args>
    Http::Response post(const std::string &target, const Args &...args)
    {
        _req = Http::Request{http::verb::post, target, _version};
        return request(target, args...);
    }

    template <typename ...Args>
    Http::Response request(const std::string &target, const Args &...args)
    {
        SPDLOG_INFO("target:[{}]", target);
        _req.set(http::field::host, _host);
        _req.set(http::field::user_agent, BOOST_BEAST_VERSION);
        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        SPDLOG_INFO("args [{}:{}]", arr.size(), sizeof...(args));
        boost::ignore_unused(arr);
        _req.version(_version);
        _req.method(http::verb::post);
        _req.keep_alive(true);

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
            boost::replace_all(v, "+", "%2B");
            boost::replace_all(v, " ", "%20");
            boost::replace_all(v, "/", "%2F");
            boost::replace_all(v, "?", "%3F");
            boost::replace_all(v, "%", "%25");
            boost::replace_all(v, "#", "%23");
            boost::replace_all(v, "&", "%26");
            boost::replace_all(v, "=", "%3D");
            param += v;
        }
        if (param.size() > 0) {
            param[0] = '?';
        }

        _req.target(target + param);
        SPDLOG_INFO("target:[{}]", std::string(_req.target()));
        // _req.set(http::field::tar)

        _req.set(http::field::body, _body);
        _req.prepare_payload();

        Http::Response resp;
        int retry = 0;
        if (_req.keep_alive()) {
            retry = 1;
        }
        do {
            int wb, rb;
            try {
                if (_enable_ssl) {
                    SPDLOG_INFO("send https");
                    wb = http::write(_ssl_stream, _req);
                    rb = http::read(_ssl_stream, _buffer, resp);
                }
                else {
                    SPDLOG_INFO("send http");
                    wb = http::write(_stream, _req);
                    rb = http::read(_stream, _buffer, resp);
                }
                SPDLOG_INFO("write bytes {}", wb);
                SPDLOG_INFO("read bytes {}", rb);
                SPDLOG_INFO("read body [{}]", resp.body());
                break;
            }
            catch (beast::system_error &e) {
                SPDLOG_ERROR("send error [{}]", e.what());
                if (--retry >= 0) {
                    reconnect();
                }
                else {
                    break;
                }
            }
        } while(retry >= 0);
        return resp;
    }
    ~HttpClient();

    bool ssl() const
    {
        return _enable_ssl;
    }
private:
    // boost::asio::ip::tcp::socket _socket;
    int _version;
    std::string _host;
    uint16_t _port;
    net::io_context _ctx;
    net::ip::tcp::resolver _resolver;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;
    Http::UrlParam _url_param;
    Http::StringBody _body;
    Http::HeadParam _head_param;
    Http::Request _req;
    // ssl
    net::ssl::context _ssl_ctx;
    // beast::ssl_stream<beast::tcp_stream> _ssl_stream;
    beast::ssl_stream<beast::tcp_stream> _ssl_stream;
    bool _enable_ssl;
    bool _connected;
    
private:
    Http::Response request(Http::Request &&req);
    void setParam(const Http::UrlParam &url_param);
    void setParam(const Http::HeadParam &url_param);
    void setParam(const Http::StringBody &body);
};
