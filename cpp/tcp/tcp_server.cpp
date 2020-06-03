#include "tcp_server.h"

TcpServer::TcpServer(const tcp::endpoint& endpoint)
  : _ctx(_inner_ctx)
  , _acceptor(_ctx, endpoint)
{
    do_accept();
}

TcpServer::TcpServer(net::io_context& io_context,
    const tcp::endpoint& endpoint)
  : _ctx(io_context)
  , _acceptor(_ctx, endpoint)
{
    do_accept();
}

void TcpServer::do_accept()
{
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (ec) {
                // SPDLOG_ERROR("accept error [{}]", ec.message());
                return;
            }
            // SPDLOG_INFO("new client [{}]", socket.remote_endpoint());
            auto connection_ptr = std::make_shared<TcpConnection>(_ctx, std::move(socket));
            // auto api_ptr = std::make_shared<${framework.service_api_class_name}>(_io_context, connection_ptr);
            // api_ptr->init();

            do_accept();
        });
}
