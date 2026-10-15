#pragma once

#include "session.hpp"
#include "utils/uuid.hpp"
#include <memory>
#include <unordered_map>
#include <mutex>

class SessionRegistry {
public:
  explicit SessionRegistry();


  std::shared_ptr<Session> add(utils::uuid id , std::shared_ptr<Session> session);
  std::shared_ptr<Session> find(utils::uuid id);
  void remove(utils::uuid id);

  std::size_t size() const;
private:
  std::mutex mutex_;
  
  std::unordered_map<utils::uuid, Session> sessions_;
} ;
