
#pragma once

#include "utils/device_id.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <nlohmann/json_fwd.hpp>

#include <memory>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class Client;
class ClientRegistry;

class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(websocket::stream<tcp::socket> ws, ClientRegistry &registry);
  void run();

  const std::string &id() const { return id_; }
  std::shared_ptr<Client> client() const { return client_.lock(); }

private:
  enum class State {
    AwaitingHello,
    AwaitingAuth,
    Ready,
    Closed,
  };

  std::string id_;
  websocket::stream<tcp::socket> ws_;
  beast::flat_buffer buffer_;
  std::weak_ptr<Client> client_;
  bool closed_ = false;
  ClientRegistry &registry_;

  State state_ = State::AwaitingHello;

  std::string pending_nonce_;
  DeviceId pending_device_id_;
  std::string pending_hardware_;
  std::string pending_name_;
  std::string pending_device_key_;

  void read();
  void handle_message(const std::string &payload);
  void handle_hello(const nlohmann::json &message);
  void handle_auth(const nlohmann::json &message);
  void handle_ready(const std::string &type, const nlohmann::json &message);

  void write(const nlohmann::json &message);
  void fail(const std::string &reason);
  void close();
};
