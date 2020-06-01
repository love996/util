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
    nlohmann::json j;
    j["deviceId"] = 572;
    Http::StringBody body(
R"((({
  "deviceId": 572, 
  "temp": 38.9,
  "fevered": 1,
  "photoUrl": "https://middle-oss.oss-cn-shenzhen.aliyuncs.com/owl/abc.jpg", 
  "bodyPhotoUrl": "https://middle-oss.oss-cn-shenzhen.aliyuncs.com/owl/abc.jpg",
  "facePhotoUrl": "https://middle-oss.oss-cn-shenzhen.aliyuncs.com/owl/abc.jpg",
  "personId": 1244, 
  "originTemp": 37.98,
  "requestId": "adqwrfsafsdgerer"
})))");
    std::cout << body << std::endl;
    Http::HeadParam head;
    head["token"] = " SMNOQgmlbce2mLnD2XweeTfXQHw=";

    net::io_context ctx;
    auto work = net::make_work_guard(ctx);
    std::thread t([&ctx]{
            ctx.run();
            });
    for (int i = 0; i < 10000; ++i) {
        auto test_ptr = std::make_shared<HttpClient>(ctx, "10.16.35.160", 8081);
        test_ptr->async_post("/api/temp/report", head, body, read_handler);
    }
    t.join();
}
