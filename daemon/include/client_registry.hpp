
#pragma once

#include "utils/device_id.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class Client;

class ClientRegistry {
public:
  enum class Outcome {
    Created,
    Authenticated,
  };

  struct Result {
    Outcome outcome = Outcome::Created;
    std::shared_ptr<Client> client;
    bool hardware_unstable = false; 
    bool hardware_moved= false; // same device id , diff hardware
    bool hardware_reused = false; // different id , same hardware
  };

  Result register_or_get(const DeviceId &device_id, const std::string &hardware,
                         const std::string &name);
  std::shared_ptr<Client> find(const DeviceId &device_id) const;
  void remove(const DeviceId &device_id);
  std::size_t size() const;

private:
  mutable std::mutex mutex_;
  std::unordered_map<DeviceId, std::shared_ptr<Client>> registry_;
  std::unordered_map<std::string, DeviceId> hardware_index_;
};
