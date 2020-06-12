#include <iostream>
using namespace std;

#include "util/output.h"
#include "curl_http/http_session.h"

int main()
{
    spdlog::set_level(spdlog::level::trace);
    Http::StringResponse resp;
    // int resp;
    auto client_ptr = std::make_shared<HttpClient>();
    client_ptr->get("https://blog.csdn.net/zww0815/article/details/51275266", resp);
    SPDLOG_DEBUG(resp.body);
    spdlog::shutdown();
}
