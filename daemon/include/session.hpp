#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(tcp::socket socket);
  void run();

  uint32_t id;

private:
  websocket::stream<tcp::socket> ws_;
};
