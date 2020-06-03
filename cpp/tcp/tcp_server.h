#pragma once

#include <cstdlib>
#include <memory>
#include <set>

#include "tcp_connection.h"

class TcpServer
{
public:
    TcpServer(net::io_context& io_context,
        const tcp::endpoint& endpoint);

    TcpServer(const tcp::endpoint& endpoint);

private:
    void do_accept();
    net::io_context _inner_ctx;
    net::io_context &_ctx;
    tcp::acceptor _acceptor;
};
