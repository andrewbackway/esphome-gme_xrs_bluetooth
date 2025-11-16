#include "at_parser.h"
#include <cctype>

namespace esphome {
namespace xrs_radio {

std::string ATParser::extract_payload(const std::string &line, const std::string &prefix) {
  auto pos = line.find(prefix);
  if (pos == std::string::npos)
    return "";
  pos += prefix.size();
  std::string payload = line.substr(pos);

  // Trim leading/trailing whitespace.
  size_t start = 0;
  size_t end = payload.size();
  while (start < end && std::isspace(static_cast<unsigned char>(payload[start])))
    ++start;
  while (end > start && std::isspace(static_cast<unsigned char>(payload[end - 1])))
    --end;
  return payload.substr(start, end - start);
}

std::vector<std::string> ATParser::split_args(const std::string &payload) {
  std::vector<std::string> result;
  std::string current;
  bool in_quotes = false;

  for (char c : payload) {
    if (c == '\"') {
      in_quotes = !in_quotes;
      current.push_back(c);
    } else if (c == ',' && !in_quotes) {
      size_t start = 0;
      size_t end = current.size();
      while (start < end && std::isspace(static_cast<unsigned char>(current[start])))
        ++start;
      while (end > start && std::isspace(static_cast<unsigned char>(current[end - 1])))
        --end;
      result.push_back(current.substr(start, end - start));
      current.clear();
    } else {
      current.push_back(c);
    }
  }

  if (!current.empty()) {
    size_t start = 0;
    size_t end = current.size();
    while (start < end && std::isspace(static_cast<unsigned char>(current[start])))
      ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(current[end - 1])))
      --end;
    result.push_back(current.substr(start, end - start));
  }

  return result;
}

}  // namespace xrs_radio
}  // namespace esphome
