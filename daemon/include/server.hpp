#pragma once
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string>
#include "client_registry.hpp"

using boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

struct ValidateInfo {
  bool ok;
  std::string name; // client / session name
  std::string message; // for logs and error
};

class Server : public std::enable_shared_from_this<Server> {
public:
  Server(boost::asio::io_context &ioc, const std::string &address,
         uint16_t port);
  void run();

private:
  void try_accept();
  void start_upgrade(tcp::socket socket); // whether to accept as session
  ValidateInfo validate_http(
      const boost::beast::http::request<boost::beast::http::string_body> &req);

  boost::asio::io_context &ioc_;
  tcp::acceptor acceptor_;

  ClientRegistry client_registry_;
  
};
