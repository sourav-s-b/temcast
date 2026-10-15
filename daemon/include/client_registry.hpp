#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include "utils/uuid.hpp"

class Client;

class ClientRegistry {
public:
  std::shared_ptr<Client> find(utils::uuid client_id);
  std::shared_ptr<Client> get_or_create(utils::uuid client_id);
  std::shared_ptr<Client> create(utils::uuid client_id);
  void remove(utils::uuid client_id);

private:
  std::mutex mutex_;
  std::unordered_map<utils::uuid ,std::shared_ptr<Client>> registry_;
};
