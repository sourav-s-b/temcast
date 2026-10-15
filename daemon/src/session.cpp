#include "session.hpp"
#include <spdlog/spdlog.h>

Session::Session(websocket::stream<tcp::socket> ws)
    : ws_(std::move(ws)) {
  id_ = utils::geneate_id();
}

void Session::run() {
  spdlog::info("Session started");
  try_read();
}

void Session::try_read() {
  ws_.async_read(buffer_, [self = shared_from_this()](
                              beast::error_code ec, std::size_t payload_size) {
    if (ec) {
      self->on_close();
      return;
    }

    std::string msg = beast::buffers_to_string(self->buffer_.data());

    spdlog::info("Recieved message: {}", msg);

    self->buffer_.consume(payload_size);

    self->try_read();
  });
}

void Session::on_close() {
  if (closed_)
    return;
  closed_ = true;
  spdlog::info("Session ended id: {}", id);
}
