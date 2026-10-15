#include "session.hpp"

#include "client.hpp"
#include "client_registry.hpp"
#include "protocol.hpp"
#include "utils/crypto.hpp"
#include "utils/uuid.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace {

std::string make_session_id() {
  utils::uuid raw = utils::geneate_id();
  std::string bytes(reinterpret_cast<const char *>(&raw), sizeof(raw));
  return crypto::to_hex(bytes);
}

} // namespace

Session::Session(websocket::stream<tcp::socket> ws, ClientRegistry &registry)
    : id_(make_session_id()), ws_(std::move(ws)), registry_(registry) {}

void Session::run() {
  spdlog::info("session {} opened", id_);
  read();
}

void Session::read() {
  ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec,
                                                      std::size_t bytes) {
    if (ec) {
      if (ec != websocket::error::closed) {
        spdlog::warn("session {} read error: {}", self->id_, ec.message());
      }
      self->close();
      return;
    }

    if (bytes > proto::kMaxMessageBytes) {
      self->fail("message exceeds maximum size");
      return;
    }

    std::string payload = beast::buffers_to_string(self->buffer_.data());
    self->buffer_.consume(bytes);

    try {
      self->handle_message(payload);
    } catch (const std::exception &e) {
      self->fail(e.what());
      return;
    }

    if (self->state_ != State::Closed) {
      self->read();
    }
  });
}

void Session::handle_message(const std::string &payload) {
  nlohmann::json message = proto::decode(payload);
  std::string type = proto::message_type(message);

  switch (state_) {
  case State::AwaitingHello:
    if (type != proto::kHello) {
      throw std::runtime_error("expected hello");
    }
    handle_hello(message);
    return;
  case State::AwaitingAuth:
    if (type != proto::kAuth) {
      throw std::runtime_error("expected auth");
    }
    handle_auth(message);
    return;
  case State::Ready:
    handle_ready(type, message);
    return;
  case State::Closed:
    return;
  }
}

void Session::handle_hello(const nlohmann::json &message) {
  if (proto::require_integer(message, proto::kFieldVersion) !=
      proto::kVersion) {
    fail("unsupported protocol version");
    return;
  }

  pending_device_id_ = proto::require_string(message, proto::kFieldDeviceId);
  pending_hardware_ = proto::optional_string(message, proto::kFieldHardware);
  pending_name_ = proto::optional_string(message, proto::kFieldName);

  std::string raw_id;
  if (!crypto::from_hex(pending_device_id_, raw_id) ||
      raw_id.size() != crypto::kPublicKeyBytes) {
    fail("malformed device id");
    return;
  }
  pending_device_key_ = std::move(raw_id);
  pending_device_id_ = crypto::to_hex(pending_device_key_);
  if (pending_name_.size() > proto::kMaxNameLen) {
    fail("name too long");
    return;
  }
  if (pending_hardware_.size() > proto::kMaxHardwareLen) {
    fail("hardware fingerprint too long");
    return;
  }
  if (pending_name_.empty()) {
    pending_name_ = "unnamed device";
  }

  pending_nonce_ = crypto::to_hex(crypto::random_bytes(crypto::kNonceBytes));

  bool known = registry_.find(pending_device_id_) != nullptr;

  state_ = State::AwaitingAuth;

  write(nlohmann::json{
      {"t", proto::kChallenge},
      {"v", proto::kVersion},
      {"nonce", pending_nonce_},
      {"known", known},
  });
}

void Session::handle_auth(const nlohmann::json &message) {
  std::string proof;
  if (!crypto::from_hex(proto::require_string(message, proto::kFieldProof),
                        proof) ||
      proof.size() != crypto::kSignatureBytes) {

    fail("malformed proof");
    return;
  }

  const std::string signed_bytes =
      crypto::auth_payload(pending_device_id_, pending_nonce_);

  if (!crypto::verify_signature(pending_device_key_, signed_bytes, proof)) {
    spdlog::warn("session {}: bad proof from {}", id_, pending_device_id_);
    fail("signature verification failed");
    return;
  }

  auto result = registry_.register_or_get(pending_device_id_, pending_hardware_,
                                          pending_name_);

  result.client->sessions().add(id_, shared_from_this());
  client_ = result.client;
  state_ = State::Ready;

  nlohmann::json welcome{
      {"t", proto::kWelcome},
      {"v", proto::kVersion},
      {"client_id", result.client->id()},
      {"name", result.client->name()},
      {"session_id", id_},
      {"known", result.outcome == ClientRegistry::Outcome::Authenticated},
  };

 if (result.hardware_unstable) {
    welcome[proto::kFieldNote] = "hardware fingerprint changed repeatedly";
  } else if (result.hardware_moved) {
    welcome[proto::kFieldNote] = "hardware fingerprint changed for this device";
  } else if (result.hardware_reused) {
    welcome[proto::kFieldNote] =
        "hardware previously seen under a different device id";
  }

  write(welcome);
}

void Session::handle_ready(const std::string &type,
                           const nlohmann::json &message) {
  spdlog::info("session {} received '{}'", id_, type);
}

void Session::write(const nlohmann::json &message) {
  auto payload = std::make_shared<std::string>(proto::encode(message));

  ws_.text(true);
  ws_.async_write(
      boost::asio::buffer(*payload),
      [self = shared_from_this(), payload](beast::error_code ec, std::size_t) {
        if (ec) {
          self->close();
          return;
        }
        if (self->state_ == State::Closed) {
          self->close();
        }
      });
}

void Session::fail(const std::string &reason) {
  spdlog::warn("session {} rejected {}", id_, reason);
  state_ = State::Closed;
  write(nlohmann::json{
      {"t", proto::kError},
      {"reason", reason},
  });
}

void Session::close() {
  if (closed_) {
    return;
  }

  closed_ = true;
  state_ = State::Closed;

  beast::error_code ignored;
  ws_.next_layer().shutdown(tcp::socket::shutdown_both, ignored);
  ws_.next_layer().close(ignored);

  if (auto client = client_.lock()) {
    client->sessions().remove(id_);
  }

  spdlog::info("session {} closed", id_);
}
