#include "http/http_server.h"

#include "util/output.h"
Http::StringResponse hello(const beast::error_code &ec, const Http::StringRequest &req)
{
    if (ec) {
        CERR << ec.message();
    }
    Http::StringResponse resp{http::status::ok, req.version()};
    resp.body() = "hello";
    resp.prepare_payload();
    return resp;
}

int main()
{
    HttpServer server("0.0.0.0", 12345);
    server.get("/hello", hello);
    server.run();
}
