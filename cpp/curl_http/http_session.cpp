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
    connect();
}

HttpSessionImpl::HttpSessionImpl()
    :HttpSessionImpl("", 0)
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
    std::string str_param;
    auto curl = _curl_ptr.get();
    // url_param
    for (auto &[k, v] : url_param) {
        str_param += "&";
        str_param += k;
        str_param += "=";
        auto convert_v = curl_easy_escape(curl, v.c_str(), v.size());
        if (convert_v) {
            str_param += convert_v;
            curl_free(convert_v);
        }
        else {
            SPDLOG_ERROR("convert error[{}][{}]", k, v);
        }
    }
    if (str_param.size() > 0) {
        str_param[0] = '?';
    }
    _target += str_param;
}


void HttpSessionImpl::setParam(const Http::HeadParam &head_param)
{
    struct curl_slist *header_list = nullptr;
    struct curl_slist *header_line = nullptr;
    Defer f([&]{
        if (header_list) {
            curl_slist_free_all(header_list);
        }
    });
    // header
    std::string buffer;
    for (auto &[k, v] : head_param) {
        // snprintf(_buffer, sizeof(_buffer), "%s:%s", k.c_str(), v.c_str());
        buffer.clear();
        buffer += k;
        buffer += ":";
        buffer += v;
        header_line = curl_slist_append(header_list, buffer.c_str());
        if (header_line) {
            header_list = header_line;
        }
        else {
            CERR << "make header error";
            return;
        }
    }
    auto curl = _curl_ptr.get();
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

}

void HttpSessionImpl::setParam(const Http::FormDataParam &form_data_param)
{
    auto curl = _curl_ptr.get();
    auto mime_ptr = std::shared_ptr<curl_mime>(curl_mime_init(curl), curl_mime_free);
    auto mime = mime_ptr.get();
    for (auto &[k, v] : form_data_param) {
        if (v.value.size()) {
            auto part = curl_mime_addpart(mime);
            curl_mime_name(part, k.c_str());
            curl_mime_data(part, v.value.c_str(), v.value.size());

        }
        else if (v.filename.size()) {
            auto part = curl_mime_addpart(mime);
            curl_mime_name(part, k.c_str());
            curl_mime_filedata(part, v.value.c_str());
            curl_mime_filename(part, k.c_str());
        }
        else {
            CERR << "error key:" << k;
        }
    }
}

void HttpSessionImpl::setParam(const Http::StringBody &body)
{
    auto curl = _curl_ptr.get();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
}

void HttpSessionImpl::setParam(Http::StringResponse &resp)
{
    SPDLOG_DEBUG("string resp");
    _stringresp_ptr = &resp;
}

void HttpSessionImpl::setParam(Http::FileResponse &resp)
{
    SPDLOG_DEBUG("file resp");
    _fileresp_ptr = &resp;
}
std::string HttpSessionImpl::getFilename() const
{
    // TODO
    assert(false && "not impl");
    return "";
}

size_t HttpSessionImpl::onHeadResponse(char *buf, size_t size, size_t n, void *lp)
{
    if (!buf || !lp) return 0;
    SPDLOG_DEBUG("head response [{}] [{}]", buf, size * n);
    // TODO
    SPDLOG_WARN("need parser header[{}]", buf);
    return 0;
}
size_t HttpSessionImpl::onStringResponse(char *buf, size_t size, size_t n, void *lp)
{
    SPDLOG_DEBUG("string response [{}] [{}]", buf, size * n);
    if (!buf || !lp) return 0;
    auto obj_ptr = static_cast<HttpSessionImpl*>(lp) ;
    obj_ptr->_stringresp_ptr->body.append(buf, size * n);
    return size * n;
}
size_t HttpSessionImpl::onFileResponse(char *buf, size_t size, size_t n, void *lp)
{
    SPDLOG_DEBUG("file response [{}] [{}]", buf, size * n);
    if (!buf || !lp) return 0;
    auto obj_ptr = static_cast<HttpSessionImpl*>(lp) ;
    auto &ofs = obj_ptr->_ofs;
    if (!ofs.is_open()) {
        ofs.open(obj_ptr->getFilename());
    }
    ofs.write(buf, size * n);
    if (ofs.good()) {
        return size * n;
    }
    return 0;
}
