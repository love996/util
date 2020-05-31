#include <iostream>
using namespace std;
#include "util/output.h"

#include "http/http_session.h"

int main()
{
    // HttpSession client("www.baidu.com", 443, true);
    HttpSession client("blog.csdn.net", 443, true);
    
    try {
        auto resp = client.get("/zww0815/article/details/51275266");
        COUT << resp.body();
        resp = client.get("/zww0815/article/details/51275266");
        COUT << resp.body();
        // client.disconnect();
    }
    catch (std::exception &e) {
        CERR << e.what();
        throw;
    }
}
