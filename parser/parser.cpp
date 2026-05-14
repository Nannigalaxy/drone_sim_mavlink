#include "parser.h"

#include <chrono>
#include <fstream>
#include <iostream>

extern "C" {
#include "../external/c_library_v2/common/mavlink.h"
}

void Parser::parse_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);

  if (!file.is_open()) {
    std::cout << "Failed to open file" << std::endl;

    return;
  }

  mavlink_message_t msg;
  mavlink_status_t status;

  uint8_t byte;

  uint64_t timestamp_us = 0;

  while (file.read(reinterpret_cast<char *>(&byte), 1)) {
    if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status)) {
      TelemetryMessage tm;

      // synthetic timing
      tm.timestamp_us = timestamp_us;

      tm.message = msg;

      messages.push_back(tm);

      // increment fake clock
      timestamp_us += 10000;
    }

    parse_errors = status.packet_rx_drop_count;
  }

  file.close();

  std::cout << "Parsed messages: " << messages.size() << std::endl;

  std::cout << "Parse errors: " << parse_errors << std::endl;
}