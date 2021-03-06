// #include "http_session.h"
#include <string>
#include "util/output.h"
#include "util/defer.h"
#include "http_session.h"
#include <boost/filesystem.hpp>

constexpr int buffer_size = 128;


// using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;

HttpSessionImpl::HttpSessionImpl(const std::string &host, uint16_t port)
    : 
    // _version{11}
    // , _host(host)
    // , _port(port)
    // , _keep_alive(true)
    _stringresp_ptr{nullptr}
    , _fileresp_ptr{nullptr}
    , _curl{nullptr}
{
    // init();
}

HttpSessionImpl::HttpSessionImpl()
    :HttpSessionImpl("", 0)
{
}

void HttpSessionImpl::init()
{
    SPDLOG_INFO("init curl");
    // destroy();
    if (_curl) {
        curl_easy_reset(_curl);
    }
    else {
        _curl = curl_easy_init();
        if (!_curl) {
            THROW_HTTP("init curl fail");
        }
    }
    if (_ofs.is_open()) {
        _ofs.close();
    }
    _stringresp_ptr = nullptr;
    _fileresp_ptr = nullptr;
    if (_buffer.capacity() != buffer_size) {
        _buffer.reserve(buffer_size);
    }
    // _buffer.clear();
}

void HttpSessionImpl::destroy()
{
    if (_curl) {
        curl_easy_cleanup(_curl);
        _curl = nullptr;
    }
}

HttpSessionImpl::~HttpSessionImpl()
{
    COUT << "disconnect";
    destroy();
}


void HttpSessionImpl::setParam(const Http::UrlParam &url_param)
{
    _buffer.clear();
    auto convert_param = curl_easy_escape(_curl, _target.c_str(), _target.size());
    if (!convert_param) {
        THROW_HTTP("target:" +  _target);
    }
    _target = convert_param;
    curl_free(convert_param);
    // url_param
    for (auto &[k, v] : url_param) {
        _buffer += "&";
        convert_param = curl_easy_escape(_curl, k.c_str(), k.size());
        if (!convert_param) {
            THROW_HTTP("param key:" + k + " value:" + v);
        }

        _buffer += convert_param;
        curl_free(convert_param);

        _buffer += "=";

        if (v.size() > 0) {
            convert_param = curl_easy_escape(_curl, v.c_str(), v.size());
            if (!convert_param) {
                THROW_HTTP("param key:" + k + " value:" + v);
            }

            _buffer += convert_param;
            curl_free(convert_param);
        }
    }
    if (_buffer .size() > 0) {
        _buffer[0] = '?';
    }
    _target += _buffer;
}


void HttpSessionImpl::setParam(const Http::HeadParam &head_param)
{
    _head_param = head_param;
}

struct curl_slist *HttpSessionImpl::makeHeadParam()
{
    struct curl_slist *header_list = nullptr;
    struct curl_slist *header_line = nullptr;
    // header
    for (auto &[k, v] : _head_param) {
        // snprintf(_buffer, sizeof(_buffer), "%s:%s", k.c_str(), v.c_str());
        _buffer.clear();
        _buffer += k;
        _buffer += ": ";
        _buffer += v;
        SPDLOG_INFO("header[{}]", _buffer);
        header_line = curl_slist_append(header_list, _buffer.c_str());
        if (header_line) {
            header_list = header_line;
        }
        else {
            CERR << "make header error";
            return header_list;
        }
    }
    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, header_list);
    return header_list;
}

void HttpSessionImpl::setParam(const Http::FormDataParam &form_data_param)
{
    auto mime_ptr = std::shared_ptr<curl_mime>(curl_mime_init(_curl), curl_mime_free);
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
    SPDLOG_DEBUG("set string body");
    if (CURLE_OK != curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, body.c_str())) {
        THROW_HTTP("set string body error");
    }
}

void HttpSessionImpl::setParam(Http::StringResponse &resp)
{
    SPDLOG_DEBUG("set string resp");
    resp.body.clear();
    _stringresp_ptr = &resp;
}

void HttpSessionImpl::setParam(Http::FileResponse &resp)
{
    SPDLOG_DEBUG("set file resp");
    _fileresp_ptr = &resp;
    // 续传
    _head_param["Range"] = "bytes=" + std::to_string(resp.size) + "-";
    if (resp.size > 0) {
        auto filename = getFilename();
        SPDLOG_DEBUG("filename:{}", filename);
        _ofs.open(filename, std::ios::in | std::ios::out);
        _ofs.seekp(resp.size);
    }
    else {
        _ofs.open(getFilename(), std::ios::out);
    }
}
std::string HttpSessionImpl::getFilename() const
{
    assert(_fileresp_ptr && "file pointer is null");
    if (_fileresp_ptr->filename.size()) {
        return _fileresp_ptr->filename;
    }

    namespace fs = boost::filesystem;
    fs::path p;
    p = _target;
    return _fileresp_ptr->filename = p.filename().string();
}

size_t HttpSessionImpl::onHeadResponse(char *buf, size_t size, size_t n, void *lp)
{
    if (!buf || !lp) return 0;
    // SPDLOG_DEBUG("head response [{}] [{}]", buf, size * n);
    // TODO
    // SPDLOG_WARN("need parser header[{}]", buf);
    return size * n;
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
    SPDLOG_DEBUG("file [{}]", size * n);
    COUT << "file:" << size * n;
    if (!buf || !lp) return 0;
    auto obj_ptr = static_cast<HttpSessionImpl*>(lp) ;
    auto &ofs = obj_ptr->_ofs;
    ofs.write(buf, size * n);
    if (ofs.good()) {
        obj_ptr->_fileresp_ptr->size += size * n;
        return size * n;
    }
    return 0;
}

size_t  write_data (void  *ptr, size_t  size, size_t  nmemb, FILE  *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
