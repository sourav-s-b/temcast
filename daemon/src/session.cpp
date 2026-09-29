#include "session.hpp"
#include <spdlog/spdlog.h>
Session::Session(tcp::socket socket) : ws_(std::move(socket)) {}

void Session::run() {
  ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
    if (ec) {
      spdlog::error("Websocket error: {}", ec.message());
      return;
    }

    auto message = std::make_shared<std::string>("Hello from TemCast");

    self->ws_.async_write(
        boost::asio::buffer(*message),
        [self, message](beast::error_code ec, std::size_t bytes_transferred) {
          if (!ec) {
            spdlog::info("Sent test data");
          }
        });
  });
}
