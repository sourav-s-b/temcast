#include "bencode.hpp"
#include <cctype>
#include <stdexcept>
#include <string>
#include <variant>

namespace {
thread_local int g_depth = 0;

struct DepthGuard {
  DepthGuard() {
    if (++g_depth > kMaxDepth){
      --g_depth;
      throw std::runtime_error("Bencode nesting too deep");
    }
  }
  ~DepthGuard() { --g_depth; }
};
} // namespace

BencodeNode parse(const std::string &buffer, size_t &index) {
  DepthGuard guard;
  if (index >= buffer.size()) {
    throw std::runtime_error("Unexpected end of buffer while parsing");
  }

  char current = buffer[index];

  if (std::isdigit(static_cast<unsigned char>(current))) {
    return parse_string(buffer, index);
  } else if (current == 'i') {
    return parse_integer(buffer, index);
  } else if (current == 'l') {
    return parse_list(buffer, index);
  } else if (current == 'd') {
    return parse_dict(buffer, index);
  }

  throw std::runtime_error(std::string("Unexpected character ") + current);
}

BencodeNode parse_integer(const std::string &buffer, size_t &index) {
  size_t end_pos = buffer.find('e', index);

  if (end_pos == std::string::npos) {
    throw std::runtime_error(
        "Invalid Bencode integer , couldn't find end character e");
  }

  index++;
  long long num;
  try {
    std::string s = buffer.substr(index, end_pos - index);
    long long num = std::stoll(s);
  } catch (const std::invalid_argument &e) {
    throw std::runtime_error("Invalid Bencode integer: non numeric characters");
  }
  index = end_pos + 1;

  BencodeNode node;
  node.data = num;

  return node;
}

BencodeNode parse_string(const std::string &buffer, size_t &index) {
  size_t colon_pos = buffer.find(':', index);

  if (colon_pos == std::string::npos) {
    throw std::runtime_error(
        "Invalid Bencode string , couldn't find character ':'");
  }

  long long length = std::stoll(buffer.substr(index, colon_pos - index));

  if (length < 0 || static_cast<std::size_t>(length) > kMaxStringBytes) {
    throw std::runtime_error("Invalid Bencode string: bad length");
  }
  if (index + length > buffer.size()) {
    throw std::runtime_error("Invalid Bencode string: length out of bounds");
  }

  std::string data =
      buffer.substr(colon_pos + 1, static_cast<std::size_t>(length));
  index = colon_pos + 1 + static_cast<std::size_t>(length);

  BencodeNode node;
  node.data = data;
  return node;
}

BencodeNode parse_list(const std::string &buffer, size_t &index) {
  index += 1;

  BencodeList list;

  while (index < buffer.size() && buffer[index] != 'e') {
    BencodeNode node = parse(buffer, index);
    list.push_back(node);
  }

  if (index >= buffer.size()) {
    throw std::runtime_error(
        "Invalid Bencode list: missing closing 'e' marker.");
  }

  index += 1;
  BencodeNode result;
  result.data = list;

  return result;
}

BencodeNode parse_dict(const std::string &buffer, size_t &index) {
  index += 1;

  BencodeDict dict;
  while (index < buffer.size() && buffer[index] != 'e') {
    if (!std::isdigit(buffer[index])) {
      throw std::runtime_error("Invalid Bencode key, must be string");
    }

    BencodeNode key = parse_string(buffer, index);
    std::string key_str = std::get<std::string>(key.data);

    BencodeNode data = parse(buffer, index);
    dict[key_str] = data;
  }
  if (index >= buffer.size()) {
    throw std::runtime_error(
        "Invalid Bencode dictionary: missing closing 'e' marker.");
  }
  index += 1;

  BencodeNode node;
  node.data = dict;

  return node;
}

std::string encode(const BencodeNode &node) {
  if (std::holds_alternative<long long>(node.data)) {

    return "i" + std::to_string(std::get<long long>(node.data)) + "e";
  } else if (std::holds_alternative<std::string>(node.data)) {

    std::string s = std::get<std::string>(node.data);
    return std::to_string(s.length()) + ":" + s;
  } else if (std::holds_alternative<BencodeList>(node.data)) {

    std::string s = "l";

    for (const auto &item : std::get<BencodeList>(node.data)) {
      s += encode(item);
    }

    return s + "e";
  } else if (std::holds_alternative<BencodeDict>(node.data)) {
    std::string s = "d";

    for (const auto &[key, val] : std::get<BencodeDict>(node.data)) {
      s += std::to_string(key.length()) + ":" + key;
      s += encode(val);
    }

    return s + "e";
  }

  throw std::runtime_error("Invalid Bencode");
}
