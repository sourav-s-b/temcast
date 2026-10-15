#include "session_registry.hpp"

#include "session.hpp"

std::shared_ptr<Session>
SessionRegistry::add(const std::string &id, std::shared_ptr<Session> session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_[id] = std::move(session);
  return session;
}

std::shared_ptr<Session> SessionRegistry::find(const std::string &id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = sessions_.find(id);
  if (it == sessions_.end()) {
    return nullptr;
  }
  return it->second;
}

void SessionRegistry::remove(const std::string &id) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.erase(id);
}

std::size_t SessionRegistry::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return sessions_.size();
}
