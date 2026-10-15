
#include "protocol.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace proto {

std::string encode(const nlohmann::json &message) {
  std::string out = message.dump();

  if (out.size() > kMaxMessageBytes) {
    throw std::runtime_error("Message Out of Bounds: Exceeds max size");
  }

  return out;
}

nlohmann::json decode(const std::string &payload) {
  if (payload.size() > kMaxMessageBytes) {
    throw std::runtime_error("Message Out of Bounds: Exceeds max size");
  }

  nlohmann::json message = nlohmann::json::parse(payload);

  if (!message.is_object()) {
    throw std::runtime_error("expected a JSON object");
  }

  if (!message.contains(kFieldType) || !message.at(kFieldType).is_string()) {
    throw std::runtime_error("missing or non-string \"t\" field");
  }

  return message;
}


std::string message_type(const nlohmann::json &message) {
  return message.at(kFieldType).get<std::string>();
}

std::string require_string(const nlohmann::json &message, const char *key) {
  if (!message.contains(key) || !message.at(key).is_string()) {
    throw std::runtime_error(std::string("missing or non-string field: ") + key);
  }
  return message.at(key).get<std::string>();
}

std::string optional_string(const nlohmann::json &message, const char *key) {
  if (!message.contains(key) || !message.at(key).is_string()) {
    return "";
  }
  return message.at(key).get<std::string>();
}

long long require_integer(const nlohmann::json &message, const char *key) {
  if (!message.contains(key) || !message.at(key).is_number_integer()) {
    throw std::runtime_error(std::string("missing or non-integer field: ") + key);
  }
  return message.at(key).get<long long>();
}

} // namespace proto
