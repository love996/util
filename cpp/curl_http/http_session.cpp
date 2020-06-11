// #include "http_session.h"
#include <string>
#include "util/output.h"
#include "util/defer.h"
#include "http_session.h"


// using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

HttpSessionImpl::HttpSessionImpl(const std::string &host, uint16_t port)
    : _version{11}
    , _host(host)
    , _port(port)
    , _keep_alive(true)
    , _connected(false)
{
}


void HttpSessionImpl::reconnect()
{
    disconnect();
    connect();
}


void HttpSessionImpl::connect()
{
    if (_connected) return;
    _curl_ptr = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
}


void HttpSessionImpl::disconnect()
{
    if (!_connected) return ;
    _connected = false;
    _curl_ptr.reset();
    return;
}

HttpSessionImpl::~HttpSessionImpl()
{
    COUT << "disconnect";
    disconnect();
}


void HttpSessionImpl::setParam(const Http::UrlParam &url_param)
{
    _url_param = url_param;
}


void HttpSessionImpl::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

void HttpSessionImpl::setParam(const Http::FormDataParam &form_data_param)
{
    _form_data_param = form_data_param;
}

void HttpSessionImpl::setParam(const Http::StringBody &body)
{
    _string_body = body;
}

void HttpSessionImpl::setParam(Http::Response &resp)
{
    _resp_ptr = &resp;
}


void HttpSessionImpl::make_request(const std::string &target)
{
    std::string url_param;
    std::string head_param;
    for (auto &[k, v] : _head_param) {
        url_param
    }

    // url_param
    for (auto &[k, v] : _url_param) {
        param += "&";
        param += k;
        param += "=";
        Http::convert(v);
        param += v;
    }
    if (param.size() > 0) {
        param[0] = '?';
    }
    auto final_target = target + param;

    if (_body.size()) {
        _req.set(http::field::body, _body);
        _req.prepare_payload();
    }
    auto curl = _curl_ptr.get();

    curl_easy_setopt(curl, CURLOPT_URL, final_target.c_str());
    // curl_easy_setopt(curl, CURLOPT_HEADER, 1);
  
    // curl_easy_setopt(curl, CURLOPT_READFUNCTION,OnReadData);
    // curl_easy_setopt(curl, CURLOPT_READDATA, rfp);
  
    // body
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWirteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)wfp);
  
    // header
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, OnWirteData);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)hfp);
  
    
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,20);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
    _req.set(http::field::host, _host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION);
    _req.version(_version);
    _req.keep_alive(_keep_alive);
    // head
}


void HttpSessionImpl::request(http::response<Body> &resp)
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


void HttpSessionImpl::async_request(StreamType &stream, const Handler &handler)
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


size_t HttpSessionImpl::onHeadRead(char *buf, size_t size, size_t n, void *lp)
{
}
size_t HttpSessionImpl::onBodyRead(char *buf, size_t size, size_t n, void *lp)
{
}
