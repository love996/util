#include <initializer_list>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <typeinfo>

#include <curl/curl.h>

#include "common/template.h"
#include "http_common.h"
#include "util/output.h"
#include "util/defer.h"

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
//
size_t  write_data (void  *ptr, size_t  size, size_t  nmemb, FILE  *stream);

class HttpSessionImpl: public std::enable_shared_from_this<HttpSessionImpl>
{
public:
    HttpSessionImpl(const std::string &host, uint16_t port);
    HttpSessionImpl();
    // HttpSessionImpl(tcp::socket &&socket);
    ~HttpSessionImpl();

    template <typename ...Args>
    void get(const std::string &target, Args &...args)
    {
        request_proxy("GET", target, args...);
    }

    template <typename ...Args>
    void post(const std::string &target, Args &...args)
    {
        request_proxy("POST", target, args...);
    }

private:
    template <typename ...Args>
    void request_proxy(const char *method, const std::string &target, Args &...args)
    {
        static_assert((contains<Http::StringBody, Args...>::value + contains<Http::FormDataParam, Args...>::value) <= 1,
                      "too much body type");
        std::tuple<Args &...> t(args...);
        auto &resp = std::get<sizeof...(args)-1>(t);

        static_assert(contains<decltype(resp), Http::FileResponse&, Http::StringResponse&>::value,
                      "at least need a response");
        
        // curl_easy_reset(_curl);
        init();

        curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, method);

        _target = target;

        std::initializer_list<int> arr{
             (this->setParam(args), 0)...
        };
        boost::ignore_unused(arr);
        SPDLOG_INFO("url[{}]", _target);
        curl_easy_setopt(_curl, CURLOPT_URL, _target.c_str());
        auto header_list = makeHeadParam();
        Defer f([&]{
            if (header_list) {
                curl_slist_free_all(header_list);
            }   
        }); 

        SPDLOG_DEBUG("request_proxy[{}][{}][{}]",
                typeid(Http::StringResponse&).name(),
                typeid(Http::FileResponse&).name(),
                typeid(decltype((resp))).name());

        make_request(resp);
        // request(resp);
    }

private:
    // int _version;
    // std::string _host;
    // uint16_t _port;
    bool _keep_alive;
    // bool _connected;
    std::fstream _ofs;
    std::string _target;
    std::string _buffer;

    // Http::UrlParam _url_param;
    Http::HeadParam _head_param;
    // Http::FormDataParam _form_data_param;
    // Http::StringBody _string_body;
    Http::StringResponse *_stringresp_ptr;
    Http::FileResponse *_fileresp_ptr;

    CURL *_curl;

private:
    void init();
    void destroy();
    std::string getFilename() const;

    template <typename Response>
    void make_request(Response &resp)
    {
        // curl_easy_setopt(_curl, CURLOPT_HEADER, 0);
        if (!strncmp(_target.c_str(), "https", strlen("https"))) {
            // curl_easy_setopt(_curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
            // curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER,false);
            // curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST,false);
        }
        //
        // struct curl_slist *headers = NULL;
        // headers = curl_slist_append(headers, "body: empty");
        // headers = curl_slist_append(headers, "cache-control: no-cache");
        // curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);
  
        // header
        curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, HttpSessionImpl::onHeadResponse);
        curl_easy_setopt(_curl, CURLOPT_HEADERDATA, &resp.header_list);
  
        SPDLOG_DEBUG("make_request[{}][{}][{}]",
                typeid(Http::StringResponse&).name(),
                typeid(Http::FileResponse&).name(),
                typeid(decltype((resp))).name());
        if constexpr(std::is_same<Response&, Http::StringResponse&>::value) {
            // string body
            SPDLOG_DEBUG("onStringResponse");
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, HttpSessionImpl::onStringResponse);
        }
        else if constexpr(std::is_same<Response&, Http::FileResponse&>::value) {
            // file body
            SPDLOG_DEBUG("onFileResponse");
            // curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_data);
            // curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, HttpSessionImpl::onStringResponse);
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, HttpSessionImpl::onFileResponse);
        }
        else {
            static_assert(always_false<Response&>, "unknown resp type");
            // static_assert(contains<Response&, Http::StringResponse&, Http::FileResponse&>::value, "unknown resp type");
        }
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
        
        curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 20);
        SPDLOG_DEBUG("send request");
        auto curl_code = curl_easy_perform(_curl);
        if (curl_code != CURLE_OK) {
            SPDLOG_ERROR("curl_easy_perform() failed:{}\n",curl_easy_strerror(curl_code));
        }
        SPDLOG_INFO("_curl code [{}]", curl_code);
    }

    // void request(Http::StringResponse &resp);
    // void request(Http::FileResponse &resp);

    void setParam(const Http::UrlParam &);
    void setParam(const Http::HeadParam &);
    struct curl_slist *makeHeadParam();
    void setParam(const Http::FormDataParam &);
    void setParam(const Http::StringBody &);
    void setParam(Http::StringResponse &);
    void setParam(Http::FileResponse &);

    static size_t onStringResponse(char *buf, size_t size, size_t n, void *lp);
    static size_t onFileResponse(char *buf, size_t size, size_t n, void *lp);
    static size_t onHeadResponse(char *buf, size_t size, size_t n, void *lp);
};

using HttpClient = HttpSessionImpl;
