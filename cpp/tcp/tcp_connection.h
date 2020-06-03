#pragma once
#include <chrono>
#include <cstddef>
#include "common/net.h"

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using ErrorCode = boost::system::error_code;
    using WriteHandler = std::function<void(const ErrorCode&, size_t)>;
    using ReadHandler = std::function<void(const ErrorCode&, size_t)>;
    using WaitHandler = std::function<void(const ErrorCode&)>;
    using TimerHandler = std::function<void(const ErrorCode&)>;
public:
    TcpConnection(const tcp::endpoint &ep);

    TcpConnection(net::io_context& io_context, const tcp::endpoint &ep);

    TcpConnection(net::io_context& io_context, tcp::socket &&socket);

    ~TcpConnection();

    const tcp::endpoint remote_endpoint() const;

    const tcp::endpoint local_endpoint() const;

    template <typename Rep, typename Period, typename Clock>
    void timer_helper(std::shared_ptr<net::steady_timer> timer_ptr, std::chrono::time_point<Clock> tp, std::chrono::duration<Rep, Period> &&timeout, TimerHandler cb)
    {
        tp = tp + timeout;
        timer_ptr->expires_at(tp);
        auto self(this->shared_from_this());
        timer_ptr->async_wait([this, self, timer_ptr, tp, timeout, cb](const ErrorCode &ec)
            {
                cb(ec);
                timer_helper(timer_ptr, tp, timeout, cb);
            });
    }

    template <typename Rep, typename Period>
    void timer(std::chrono::duration<Rep, Period> &&timeout, TimerHandler cb)
    {
        auto timer_ptr = std::make_shared<net::steady_timer>(_ctx);
        auto tp = std::chrono::steady_clock::now();
        timer_helper(timer_ptr, tp, timeout, cb);
    }

    template <typename Rep, typename Period>
    void timer_once(std::chrono::duration<Rep, Period> &&timeout, TimerHandler cb)
    {
        auto timer_ptr = std::make_shared<net::steady_timer>(_ctx, timeout);
        auto self(this->shared_from_this());
        timer_ptr->async_wait([this, self, timer_ptr, cb](const ErrorCode &ec)
            { 
                cb(ec);
            }
        );
    }

    void async_wait_read(WaitHandler cb);

    void async_wait_write(WaitHandler cb);

    void write(const std::byte *data, size_t length);

    void async_write(const std::byte *data, size_t length, WriteHandler cb);

    void read(std::byte *data, size_t length);

    void async_read(std::byte *data, size_t length, ReadHandler cb);

    void async_read_some(std::byte *data, size_t length, ReadHandler cb);

    void close();

private:
    net::io_context _inner_ctx;
    net::io_context& _ctx;
    tcp::socket _socket;
private:
    void connect(const tcp::endpoint &ep);
};
