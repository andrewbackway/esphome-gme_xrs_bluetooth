#pragma once

#include <string>
#include <vector>

namespace esphome {
namespace xrs_radio {

// Lightweight helper for parsing simple AT notification lines.
class ATParser {
 public:
  // Extract the payload part of a line after a given prefix, trimming whitespace.
  // For example, line "+WGCHSQ: 1,40,476.425,476.425,\"CH40\"" with prefix "+WGCHSQ:"
  // yields "1,40,476.425,476.425,\"CH40\"".
  static std::string extract_payload(const std::string &line, const std::string &prefix);

  // Split a comma-separated payload into fields, keeping quoted strings intact.
  // Whitespace around fields is trimmed but quotes are preserved.
  static std::vector<std::string> split_args(const std::string &payload);
};

}  // namespace xrs_radio
}  // namespace esphome
