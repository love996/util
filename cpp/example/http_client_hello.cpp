#include "http/http_session.h"
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
using namespace std;

#include "util/output.h"

// #include "log/log.h"

void read_handler(beast::error_code const &, Http::Response const &resp)
{
    COUT << resp.body();
}

int main()
{
    net::io_context ctx;
    for (int i = 0; i < 10; ++i) {
        auto test_ptr = std::make_shared<HttpClient>(ctx, "192.168.5.128", 12345);
        test_ptr->async_get("/hello", read_handler);
    }
    ctx.run();
}
