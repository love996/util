#include <iostream>
using namespace std;

#include "util/output.h"
#include "curl_http/http_session.h"

int main()
{
    try {
        spdlog::set_level(spdlog::level::trace);
        Http::FileResponse file_resp;
        Http::StringResponse resp;
        // int resp;
        auto client_ptr = std::make_shared<HttpClient>();
        client_ptr->get("https://middle-oss.oss-cn-shenzhen.aliyuncs.com/ota/20200512/RrAvVq_Jetson_Nano_OTA-V0.0.3-20200512.zip", file_resp);

        // client_ptr.reset();
        // client_ptr = std::make_shared<HttpClient>();
        client_ptr->get("https://blog.csdn.net/zww0815/article/details/51275266", resp);
        SPDLOG_DEBUG(resp.body);
        SPDLOG_DEBUG("filename [{}] file size {}", file_resp.filename, file_resp.size);


        client_ptr.reset();
        client_ptr = std::make_shared<HttpClient>();
        Http::UrlParam param;
        param["alarmTypeCode"] = "95714%2341";
        param["alarm_source_url"] = "";
        param["alarm_time"] = "1590576073";
        param["currentLevelAngle"] = "50.0000";
        param["currentPitchAngle"] = "40.0000";
        param["deviceId"] = "0";
        param["fire_area"] = "30.0000";
        param["flag"] = "1";
        client_ptr = std::make_shared<HttpClient>();
        client_ptr->post("http://10.16.33.7:8081/robots-test/robotCommunication/handleDeviceAlarm", param,
                         resp);
        SPDLOG_DEBUG(resp.body);
    }
    catch (boost::exception &e) {
        SPDLOG_ERROR(boost::diagnostic_information(e));
        // SPDLOG_ERROR(*boost::get_error_info<string>(e));
    }
}
