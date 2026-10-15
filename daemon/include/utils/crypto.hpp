#pragma once

#include <cstddef>
#include <string>

namespace crypto {
inline constexpr std::size_t kNonceBytes = 32;
inline constexpr std::size_t kPublicKeyBytes = 32;
inline constexpr std::size_t kSignatureBytes = 64;

std::string random_bytes(std::size_t n);

std::string to_hex(const std::string &raw);
bool from_hex(const std::string &hex , std::string &out);

bool constant_time_eq(const std::string &a , const std:: string &b);

// server ops
bool verify_signature(const std::string &public_key , const std::string &message , const std::string &signature);

std::string auth_payload(const std::string &device_id_hex, const std::string &nonce_hex);


// client ops
std::string generate_secret_key();
std::string public_key_from_secret(const std::string &secret_key);
std::string sign(const std::string &secret_key , const std::string &message);
} // namespace crypt
