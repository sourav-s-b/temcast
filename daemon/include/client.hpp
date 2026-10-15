#pragma once

#include "session_registry.hpp"
#include "utils/uuid.hpp"
#include <string>


class Client {
public:
  Client();

  explicit Client(const utils::uuid id) : id_(std::move(id)) {}

  const utils::uuid &id() const { return id_; }

  const std::string &name() const { return name_; }

  SessionRegistry& registry() {
    return registry_;
  }

private:
  utils::uuid id_;
  std::string name_;

  SessionRegistry registry_;
};
