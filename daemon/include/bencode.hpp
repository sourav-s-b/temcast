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

BencodeNode parse(const std::string &buffer, size_t &index);

std::string encode(const BencodeNode &node);
