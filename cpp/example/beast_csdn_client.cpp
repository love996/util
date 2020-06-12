#include <iostream>
using namespace std;
#include "util/output.h"

#include "beast_http/http_session.h"

void readHandler(const beast::error_code &, const Http::StringResponse &resp)
{
    COUT << resp.body();
}

int main()
{
    try {
        net::io_context ctx;
        auto client_ptr = std::make_shared<HttpsClient>(ctx, "blog.csdn.net", 443);
        client_ptr->async_get("/zww0815/article/details/51275266", readHandler);
        ctx.run();
    }
    catch (std::exception &e) {
        CERR << e.what();
        throw;
    }
}
