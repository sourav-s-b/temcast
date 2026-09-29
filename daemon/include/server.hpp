#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

class Server : public std::enable_shared_from_this<Server> {
public:
  Server(boost::asio::io_context &ioc, const std::string &address,
         uint16_t port);
  void run();

private:
  void try_accept();

  boost::asio::io_context &ioc_;
  tcp::acceptor acceptor_;
};
