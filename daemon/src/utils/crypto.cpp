#include "utils/crypto.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <array>
#include <stdexcept>

namespace crypto {

std::string random_bytes(std::size_t n) {
  std::string out(n, '\0');

  if (RAND_bytes(reinterpret_cast<unsigned char *>(out.data()),
                 static_cast<int>(n)) != 1) {
    throw std::runtime_error("RAND_bytes failed");
  }
  return out;
}

std::string to_hex(const std::string &raw) {
  static const char *digits = "0123456789abcdef";
  std::string out;
  out.reserve(raw.size() * 2);
  for (char ch : raw) {
    auto byte = static_cast<unsigned char>(ch);
    out.push_back(digits[byte >> 4]);
    out.push_back(digits[byte & 0x0f]);
  }
  return out;
}

bool from_hex(const std::string &hex, std::string &out) {
  if (hex.size() % 2 != 0) {
    return false;
  }

  out.clear();
  out.reserve(hex.size() / 2);

  auto nibble = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  };

  for (std::size_t i = 0; i < hex.size(); i += 2) {
    int hi = nibble(hex[i]);
    int lo = nibble(hex[i + 1]);
    if (hi < 0 || lo < 0)
      return false;

    out.push_back(static_cast<char>((hi << 4) | lo));
  }
  return true;
}

bool constant_time_eq(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) {
    return false;
  }

  unsigned char diff = 0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
  }
  return diff == 0;
}

namespace {

EVP_PKEY *load_public_key(const std::string &raw) {
  return EVP_PKEY_new_raw_public_key(
      EVP_PKEY_ED25519, nullptr,
      reinterpret_cast<const unsigned char *>(raw.data()), raw.size());
}

EVP_PKEY *load_private_key(const std::string &raw) {
  return EVP_PKEY_new_raw_private_key(
      EVP_PKEY_ED25519, nullptr,
      reinterpret_cast<const unsigned char *>(raw.data()), raw.size());
}

} // namespace
std::string auth_payload(const std::string &device_id_hex,
                         const std::string &nonce_hex) {

  // TODO: need to change this to be not hardcoded
  return "temcast-auth-v1|" + device_id_hex + "|" + nonce_hex;
}

bool verify_signature(const std::string &public_key, const std::string &message,
                      const std::string &signature) {
  if (public_key.size() != kPublicKeyBytes ||
      signature.size() != kSignatureBytes) {
    return false;
  }

  EVP_PKEY *pkey = load_public_key(public_key);
  if (pkey == nullptr) {
    return false;
  }

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  bool ok = false;

  if (ctx != nullptr &&
      EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) == 1) {
    ok = EVP_DigestVerify(
             ctx, reinterpret_cast<const unsigned char *>(signature.data()),
             signature.size(),
             reinterpret_cast<const unsigned char *>(message.data()),
             message.size()) == 1;
  }

  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return ok;
}

std::string generate_secret_key() { return random_bytes(32); }

std::string public_key_from_secret(const std::string &secret_key) {
  EVP_PKEY *pkey = load_private_key(secret_key);
  if (pkey == nullptr) {
    throw std::runtime_error("bad private key");
  }

  std::array<unsigned char, kPublicKeyBytes> buf{};
  std::size_t len = buf.size();
  bool ok = EVP_PKEY_get_raw_public_key(pkey, buf.data(), &len) == 1;

  EVP_PKEY_free(pkey);
  if (!ok || len != kPublicKeyBytes) {
    throw std::runtime_error("could not derive public key");
  }
  return std::string(reinterpret_cast<char *>(buf.data()), buf.size());
}

std::string sign(const std::string &secret_key, const std::string &message) {
  EVP_PKEY *pkey = load_private_key(secret_key);
  if (pkey == nullptr) {
    throw std::runtime_error("bad private key");
  }

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  std::array<unsigned char, kSignatureBytes> sig{};
  std::size_t sig_len = sig.size();
  bool ok = false;

  if (ctx != nullptr &&
      EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, pkey) == 1) {
    ok = EVP_DigestSign(ctx, sig.data(), &sig_len,
                        reinterpret_cast<const unsigned char *>(message.data()),
                        message.size()) == 1;
  }

  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);

  if (!ok || sig_len != kSignatureBytes) {
    throw std::runtime_error("signing failed");
  }
  return std::string(reinterpret_cast<char *>(sig.data()), sig.size());
}
} // namespace crypto
