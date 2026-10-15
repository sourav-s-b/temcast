#pragma once
#include "utils/uuid.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class Client;

class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(websocket::stream<tcp::socket> ws);
  void run();

  const utils::uuid id() const { return id_; }

private:
  utils::uuid id_;

  websocket::stream<tcp::socket> ws_;
  beast::flat_buffer buffer_;
  std::weak_ptr<Client> client_;
  bool closed_ = false;

  void try_read();
  void on_close();
};
