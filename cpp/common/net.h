#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include "exception.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using net::ip::tcp;
// using tcp_socket = net::basic_stream_socket<tcp, net::io_context::executor_type>;

#include <spdlog/fmt/ostr.h>
inline std::ostream &operator<<(std::ostream &os, const tcp::endpoint &ep)
{
    os << ep.address().to_string() << ":" << ep.port();
    return os;
}
