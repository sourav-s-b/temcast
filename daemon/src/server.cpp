#include "server.hpp"
#include "session.hpp"
#include <spdlog/spdlog.h>

Server::Server(boost::asio::io_context &ioc, const std::string &address,
               uint16_t port)
    : ioc_(ioc),
      acceptor_(ioc,
                tcp::endpoint(boost::asio::ip::make_address(address), port)) {};

void Server::run() {
  spdlog::info("Server listening:on {}:{}",
               acceptor_.local_endpoint().address().to_string(),
               acceptor_.local_endpoint().port());

  try_accept();
}

void Server::try_accept() {
  acceptor_.async_accept(boost::asio::make_strand(ioc_),
                         [self = shared_from_this()](
                             boost::system::error_code ec, tcp::socket socket) {
                           if (!ec) {
                             auto session =
                                 std::make_shared<Session>(std::move(socket));

                             session->run();
                           } else {
                             spdlog::error("Accept error: {}", ec.message());
                           }
                           self->try_accept();
                         });
}
