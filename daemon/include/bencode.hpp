#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

struct BencodeNode;

using BencodeList = std::vector<BencodeNode>;
using BencodeDict = std::map<std::string, BencodeNode>;

struct BencodeNode {
  std::variant<long long, std::string, BencodeList, BencodeDict> data;
};

inline constexpr std::size_t kMaxStringBytes = 64*1024;
inline constexpr int kMaxDepth = 32;

BencodeNode parse(const std::string &buffer, size_t &index);
BencodeNode parse_integer(const std::string &buffer, size_t &index);
BencodeNode parse_string(const std::string &buffer, size_t &index);
BencodeNode parse_list(const std::string &buffer, size_t &index);
BencodeNode parse_dict(const std::string &buffer, size_t &index);

std::string encode(const BencodeNode &node);
