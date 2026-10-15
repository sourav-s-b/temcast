#include "client_registry.hpp"

#include "client.hpp"
#include "session_registry.hpp"

#include <spdlog/spdlog.h>

ClientRegistry::Result
ClientRegistry::register_or_get(const DeviceId &device_id,
                                const std::string &hardware,
                                const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);

  Result result;

  auto it = registry_.find(device_id);
  if (it == registry_.end()) {
    auto client = std::make_shared<Client>(device_id, hardware);
    client->set_name(name);
    registry_.emplace(device_id, client);

    result.outcome = Outcome::Created;
    result.client = std::move(client);

    if (!hardware.empty()) {
      auto hw_it = hardware_index_.find(hardware);
      if (hw_it != hardware_index_.end()) {
        // same machine but new key
        result.hardware_reused = true;
        spdlog::warn(
            "hardware {} previously registered as {}, now presenting as {}",
            hardware, hw_it->second, device_id);
      }

      hardware_index_[hardware] = device_id;
    }
    spdlog::info("registered new device {} ({})", device_id, name);
    return result;
  }

  auto client = it->second;
  result.outcome = Outcome::Authenticated;
  result.client = client;

  if (client->observe_hardware(hardware)) {
    result.hardware_moved = true;
    result.hardware_unstable = client->hardware_changes() >= 2;

    spdlog::warn(
        "device {} ({}) hardware fingerprint changed (change #{}, {} -> {}, "
        "originally {})",
        device_id, client->name(), client->hardware_changes(),
        client->registered_hardware(), hardware, client->registered_hardware());
  }
  client->set_name(name);
  client->touch();
  return result;
}

std::shared_ptr<Client> ClientRegistry::find(const DeviceId &device_id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = registry_.find(device_id);
  if (it == registry_.end()) {
    return nullptr;
  }
  return it->second;
}

void ClientRegistry::remove(const DeviceId &device_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = registry_.find(device_id);
  if (it == registry_.end()) {
    return;
  }
  hardware_index_.erase(it->second->hardware());
  registry_.erase(it);
}
std::size_t ClientRegistry::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return registry_.size();
}
