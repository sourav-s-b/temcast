#include "server.hpp"
#include "session.hpp"
#include <boost/beast/core/flat_buffer.hpp>
#include <spdlog/spdlog.h>
#include "utils/uuid.hpp"

namespace asio = boost::asio;

Server::Server(asio::io_context &ioc, const std::string &address, uint16_t port)
    : ioc_(ioc),
      acceptor_(ioc, tcp::endpoint(asio::ip::make_address(address), port)) {};

void Server::run() {
  spdlog::info("Server listening:on {}:{}",
               acceptor_.local_endpoint().address().to_string(),
               acceptor_.local_endpoint().port());

  try_accept();
}

ValidateInfo Server::validate_http(
    const beast::http::request<beast::http::string_body> &req) {

  auto header = req.find("X-TemCast");
  if (header == req.end()) {
    return {false, "None", "Doesn't contain correct headers'"};
  }

  std::string name(header->value());

  if (name.empty()) {
    return {false, "None", "X-TemCast header is empty"};
  }

  return {true, name, ""};
}

void Server::try_accept() {
  acceptor_.async_accept(asio::make_strand(ioc_),
                         [self = shared_from_this()](
                             boost::system::error_code ec, tcp::socket socket) {
                           if (ec) {
                             spdlog::error("Accept error: {}", ec.message());
                             self->try_accept(); // re-arm on failure
                             return;
                           }
                           self->start_upgrade(std::move(socket));
                           self->try_accept();
                         });
}

void Server::start_upgrade(tcp::socket socket) {
  auto sock = std::make_shared<tcp::socket>(std::move(socket));
  auto buf = std::make_shared<beast::flat_buffer>();
  auto req = std::make_shared<beast::http::request<beast::http::string_body>>();

  beast::http::async_read(
      *sock, *buf, *req,
      [self = shared_from_this(), sock, buf, req](beast::error_code ec,
                                                  std::size_t) {
        if (ec) {
          spdlog::warn("Error in parsing http {}", ec.message());
          sock->close();
          return;
        }

        ValidateInfo info = self->validate_http(*req);
        if (!info.ok) {
          spdlog::warn("Rejected: {}", info.message);

          auto res = beast::http::response<beast::http::string_body>{
              beast::http::status::bad_request, req->version()};
          res.set(beast::http::field::server, "TemCast");
          res.body() = "Not a TemCast client\n";
          res.prepare_payload();

          beast::http::serializer<false, beast::http::string_body> sr{res};
          beast::http::async_write(
              *sock, *res, [sock, res](beast::error_code ec, std::size_t) {
                beast::error_code ignored;
                sock->shutdown(tcp::socket::shutdown_both, ignored);
                sock->close(ignored);
              });
          return;
        }

        auto ws =
            std::make_shared<websocket::stream<tcp::socket>>(std::move(*sock));

        ws->async_accept(*req, [self, ws](beast::error_code ec) {
          if (ec) {
            spdlog::warn("Upgrade failed: {}", ec.message());
            ws->next_layer().close();
            return;
          }

          spdlog::info("Client connected");
          client_registry_.get_or_create();
          //auto session = std::make_shared<Session>(std::move(*ws));
         // session->run();
        });
      });
}
