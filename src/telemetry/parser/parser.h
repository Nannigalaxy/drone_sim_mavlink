#pragma once

#include <string>
#include <vector>

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

struct TelemetryMessage {
  uint64_t timestamp_us = 0;

  mavlink_message_t message;
};

class Parser {

public:
  std::vector<TelemetryMessage> messages;
  size_t parse_errors = 0;

  void parse_file(const std::string &filename);
};
