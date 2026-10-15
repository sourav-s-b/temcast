#pragma once
#include "client_registry.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string>

using boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

class Server : public std::enable_shared_from_this<Server> {
public:
  Server(boost::asio::io_context &ioc, const std::string &address,
         uint16_t port);
  void run();
  ClientRegistry &registry() { return client_registry_; }

private:
  using http_request = beast::http::request<beast::http::string_body>;

  void try_accept();
  void start_upgrade(tcp::socket socket); 

  bool validate_http(const http_request &req, std::string &error);
  void reject(const std::shared_ptr<tcp::socket> &sock, beast::http::status status, const std::string &message);

  boost::asio::io_context &ioc_;
  tcp::acceptor acceptor_;

  ClientRegistry client_registry_;
};
