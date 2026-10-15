#pragma once

#include <nlohmann/json_fwd.hpp>

#include <cstddef>
#include <string>

namespace proto {
inline constexpr long long kVersion = 1;

// for unauthenticated limits
inline constexpr std::size_t kMaxMessageBytes = 8 * 1024;
inline constexpr std::size_t kMaxNameLen = 64;
inline constexpr std::size_t kMaxHardwareLen = 256;

// message types
inline const char *kHello = "hello";
inline const char *kChallenge = "challenge";
inline const char *kAuth = "auth";
inline const char *kWelcome = "welcome";
inline const char *kError = "error";

// field names
inline const char *kFieldType = "t";
inline const char *kFieldVersion = "v";
inline const char *kFieldDeviceId = "device_id";
inline const char *kFieldHardware = "hw";
inline const char *kFieldName = "name";
inline const char *kFieldNonce = "nonce";
inline const char *kFieldKnown = "known";
inline const char *kFieldProof = "proof";
inline const char *kFieldClientId = "client_id";
inline const char *kFieldSessionId = "session_id";
inline const char *kFieldReason = "reason";
inline const char *kFieldNote = "note";


// helpers
std::string encode(const nlohmann::json &message);
nlohmann::json decode(const std::string &payload);

std::string message_type(const nlohmann::json &message);

std::string require_string(const nlohmann::json &message, const char *key);
std::string optional_string(const nlohmann::json &message, const char *key);
long long require_integer(const nlohmann::json &message, const char *key);
}
