#include "server.hpp"

#include "session.hpp"

#include <boost/beast/core/flat_buffer.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <utility>

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

bool Server::validate_http(const http_request &req, std::string &error) {
  auto header = req.find("X-TemCast-Protocol");
  if (header == req.end()) {
    error = "missing X-TemCast-Protocol header";
    return false;
  }
  if (header->value() != "1") {
    error = "unsupported protocol version: " + std::string(header->value());
    return false;
  }
  return true;
}

void Server::reject(const std::shared_ptr<tcp::socket> &sock,
                    beast::http::status status, const std::string &message) {
  auto res = std::make_shared<beast::http::response<beast::http::string_body>>(
      status, 11);

  res->set(beast::http::field::server, "TemCast");
  res->set(beast::http::field::content_type, "text/plain");
  res->keep_alive(false);
  res->body() = message;
  res->prepare_payload();

  beast::http::async_write(
      *sock, *res, [sock, res](beast::error_code ec, std::size_t) {
        beast::error_code ignored;
        sock->shutdown(tcp::socket::shutdown_both, ignored);
        sock->close(ignored);
      });
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
  auto req = std::make_shared<http_request>();

  beast::http::async_read(
      *sock, *buf, *req,
      [self = shared_from_this(), sock, buf, req](beast::error_code ec,
                                                  std::size_t) {
        if (ec) {
          spdlog::warn("Error in parsing http {}", ec.message());
          beast::error_code ignored;
          sock->close(ignored);
          return;
        }

        std::string error;
        if (!self->validate_http(*req, error)) {
          spdlog::warn("Rejected: {}", error);
          self->reject(sock, beast::http::status::bad_request, error);
          return;
        }

        auto ws =
            std::make_shared<websocket::stream<tcp::socket>>(std::move(*sock));

      
asio::dispatch(ws->get_executor(), [self, ws, req]() mutable {
          ws->async_accept(*req, [self, ws](beast::error_code ec) {
            if (ec) {
              spdlog::warn("Upgrade failed: {}", ec.message());
              beast::error_code ignored;
              ws->next_layer().close(ignored);
              return;
            }

            auto session =
                std::make_shared<Session>(std::move(*ws), self->client_registry_);
            session->run();
          });
        });
      });

}
