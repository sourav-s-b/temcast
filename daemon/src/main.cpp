#include "server.hpp"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

int main() {

  spdlog::info("Starting TemCast server...");

  try {
    boost::asio::io_context ioc;

    auto server = std::make_shared<Server>(ioc, "127.0.0.1", 6969);
    server->run();
    ioc.run();
  } catch (const std::exception &e) {
    spdlog::error("Fatal daemon error: {}", e.what());
    return 1;
  }
  return 0;
}
