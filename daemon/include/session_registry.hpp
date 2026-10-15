#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class Session;

class SessionRegistry {
public:

  std::shared_ptr<Session> add(const std::string &id,
                               std::shared_ptr<Session> session);
  std::shared_ptr<Session> find(const std::string &id) const;
  void remove(const std::string &id);

  std::size_t size() const;

private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};
