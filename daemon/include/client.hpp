#pragma once

#include "session_registry.hpp"
#include "utils/device_id.hpp"

#include <chrono>
#include <string>
#include <utility>

class Client {
public:
  Client(DeviceId device_id, std::string hardware)
      : device_id_(std::move(device_id)), registered_hardware_(hardware),hardware_(std::move(hardware)) {
    last_seen_ = std::chrono::system_clock::now();
  }

  const DeviceId &id() const { return device_id_; }
  const std::string &name() const { return name_; }
  void set_name(std::string name) { name_ = std::move(name); }

  const std::string &registered_hardware() const {
    return registered_hardware_;
  }
  const std::string &hardware() const { return hardware_; }

  std::size_t hardware_changes() const { return hardware_changes_; }
  bool observe_hardware(const std::string &hardware) {
    if (hardware.empty() || hardware == hardware_) {
      return false;
    }
    hardware_ = hardware;
    ++hardware_changes_;
    return true;
  }

  void touch() { last_seen_ = std::chrono::system_clock::now(); }
  std::chrono::system_clock::time_point last_seen() const { return last_seen_; }

  const SessionRegistry &sessions() const { return sessions_; }
  SessionRegistry &sessions() { return sessions_; }

private:
  DeviceId device_id_;
  std::string name_;
  std::string registered_hardware_;
  std::string hardware_;
  std::size_t hardware_changes_ = 0;
  std::chrono::system_clock::time_point last_seen_;

  SessionRegistry sessions_;
};
