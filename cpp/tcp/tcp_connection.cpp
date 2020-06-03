#include "tcp_connection.h"
#include <memory>

#include <boost/asio.hpp>

TcpConnection::TcpConnection(const tcp::endpoint &ep)
    : _ctx(_inner_ctx)
    , _socket(_ctx)
{
    connect(ep);
}


TcpConnection::TcpConnection(net::io_context& io_context, const tcp::endpoint &ep)
    : _ctx(io_context)
    , _socket(_ctx)
{
    connect(ep);
}

TcpConnection::TcpConnection(net::io_context& io_context, tcp::socket &&socket)
    : _ctx(io_context)
    , _socket(std::move(socket))
{
}

TcpConnection::~TcpConnection()
{
    // SPDLOG_INFO("disconnect remote_endpoint[{}]", remote_endpoint());
}

const tcp::endpoint TcpConnection::remote_endpoint() const
{
    return _socket.remote_endpoint();
}

const tcp::endpoint TcpConnection::local_endpoint() const
{
    return _socket.local_endpoint();
}

void TcpConnection::async_wait_read(WaitHandler cb)
{
    _socket.async_wait(tcp::socket::wait_read, cb);
}

void TcpConnection::async_wait_write(WaitHandler cb)
{
    _socket.async_wait(tcp::socket::wait_write, cb);
}

void TcpConnection::write(const std::byte *data, size_t length)
{
    net::write(_socket, net::buffer(data, length));
}

void TcpConnection::async_write(const std::byte *data, size_t length, WriteHandler cb)
{
    assert(cb && "callback can not be null");
    net::async_write(_socket, net::buffer(data, length), cb);
}

void TcpConnection::read(std::byte *data, size_t length)
{
    net::read(_socket, net::buffer(data, length));
}

void TcpConnection::async_read(std::byte *data, size_t length, ReadHandler cb)
{
    assert(cb && "callback can not be null");
    net::async_read(_socket, net::buffer(data, length), cb);
}

void TcpConnection::async_read_some(std::byte *data, size_t length, ReadHandler cb)
{
    assert(cb && "callback can not be null");
    _socket.async_read_some(net::buffer(data, length), cb);
}

void TcpConnection::close()
{
    // _socket.close();
    // _socket.get_executor().post(std::bind(&TcpConnection::close, this->shared_from_this()));
    net::post(_socket.get_executor(), std::bind(&TcpConnection::close, this->shared_from_this()));
}

void TcpConnection::connect(const tcp::endpoint &ep)
{
    _socket.connect(ep);
}
