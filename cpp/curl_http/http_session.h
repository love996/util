#include <initializer_list>
#include <iostream>
#include <type_traits>

#include <curl/curl.h>

#include "common/template.h"
#include "http_common.h"
#include "util/output.h"

#define SYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    void method(const std::string &target, const Args &...args) \
    {   \
        static_assert(!(contains<Http::StringResponse&, Args...>::value && \
                      contains<Http::FileResponse&, Args...>::value), \
                      "only need one response type" ); \
        static_assert(contains<Http::StringResponse&, Args...>::value || \
                      contains<Http::FileResponse&, Args...>::value, \
                      "need a response" ); \
        _req = Http::Request{http::verb::method, target, _version}; \
        request_proxy(target, args...); \
    }

#define ASYNC_HTTP_METHOD(method) \
    template <typename ...Args> \
    void async_##method(const std::string &target, const Args &...args) \
    {   \
        _req = Http::Request{http::verb::method, target, _version}; \
        async_request_proxy(target, args...); \
    }


#define HTTP_METHOD(method) \
    SYNC_HTTP_METHOD(method) \
    ASYNC_HTTP_METHOD(method)

// using HandlerMap = std::map<Http::RouteInfo, Http::RequestHandler>;
// using HandlerMapPtr = std::shared_ptr<HandlerMap>;

class HttpSessionImpl: public std::enable_shared_from_this<HttpSessionImpl>
{
public:
    HttpSessionImpl(const std::string &host, uint16_t port);
    // HttpSessionImpl(tcp::socket &&socket);
    ~HttpSessionImpl();


private:
    template <typename ...Args>
    void get(const std::string &target, Args &...args)
    {
        request_proxy(target, args...);
    }

    template <typename ...Args>
    void make_request_proxy(const std::string &target, Args &...args)
    {
        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        boost::ignore_unused(arr);
        make_request(target);
    }

    template <typename ...Args>
    void request_proxy(const std::string &target, Args &...args)
    {
        static_assert((contains<Http::StringBody, Args...>::value + contains<Http::FormDataParam, Args...>::value) <= 1,
                      "too much body type");
        _target = target;
        make_request_proxy(target, args...);
        std::tuple<Args &...> t(args...);
        auto &resp = std::get<sizeof...(args)-1>(args...);
        request(resp);
    }

private:
    int _version;
    std::string _host;
    uint16_t _port;
    bool _keep_alive;
    bool _connected;
    std::string _target;

    // Http::UrlParam _url_param;
    // Http::HeadParam _head_param;
    // Http::FormDataParam _form_data_param;
    // Http::StringBody _string_body;
    // Http::Response *_resp_ptr;
    // Http::FileResponse *_fileresp_ptr;

    std::shared_ptr<CURL> _curl_ptr;
    char _buffer[128];

    // 
    // Http::FileResponse _file_resp;
    // ssl

private:
    void reconnect();
    void connect();
    void disconnect();
    void make_request(const std::string &target) ;
    template <typename Response>
    void request(Response &resp);

    void setParam(const Http::UrlParam &);
    void setParam(const Http::HeadParam &);
    void setParam(const Http::FormDataParam &);
    void setParam(const Http::StringBody &);
    void setParam(Http::Response &resp);
    void setParam(Http::FileResponse &resp);

    static size_t onBodyResponse(char *buf, size_t size, size_t n, void *lp);
    static size_t onFileResponse(char *buf, size_t size, size_t n, void *lp);
    static size_t onHeadResponse(char *buf, size_t size, size_t n, void *lp);
};
