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
        client_ptr->get("https://download.microsoft.com/download/4/2/2/42245968-6A79-4DA7-A5FB-08C0AD0AE661/windowssdk/winsdksetup.exe", file_resp);
        SPDLOG_DEBUG("filename [{}] file size {}", file_resp.filename, file_resp.size);

        client_ptr = std::make_shared<HttpClient>();
        client_ptr->get("https://blog.csdn.net/zww817/article/details/51275266", resp);
        SPDLOG_DEBUG(resp.body);


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
