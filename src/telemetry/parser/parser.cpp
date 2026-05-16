#include "parser.h"

#include <fstream>
#include <iostream>

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

static uint64_t read_be_u64(const uint8_t *b) {
  // read 8 bytes in big-endian order and convert to uint64_t
  return ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48) |
         ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32) |
         ((uint64_t)b[4] << 24) | ((uint64_t)b[5] << 16) |
         ((uint64_t)b[6] << 8) | ((uint64_t)b[7]);
}

void Parser::parse_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);

  if (!file.is_open()) {
    std::cout << "Failed to open file" << std::endl;

    return;
  }

  mavlink_status_t status;
  mavlink_message_t msg;

  while (true) {

    // buffer for timestamp (8 bytes) + MAVLink packet (up to 263 bytes)
    uint8_t tsbuf[8];

    file.read(reinterpret_cast<char *>(tsbuf), 8);

    if (!file.good()) {
      break;
    }

    uint64_t timestamp_us = read_be_u64(tsbuf);
    bool packet_complete = false;

    while (!packet_complete && file.good()) {
      // read one byte at a time and feed to MAVLink parser until a complete
      // packet is parsed
      uint8_t byte;
      file.read(reinterpret_cast<char *>(&byte), 1);

      if (!file.good()) {
        break;
      }

      // feed byte to MAVLink parser
      if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status)) {
        TelemetryMessage tm;

        tm.timestamp_us = timestamp_us;
        tm.message = msg;
        messages.push_back(tm); // store parsed message to use later in analysis
        packet_complete = true;
      }

      parse_errors = status.packet_rx_drop_count;
    }
  }

  file.close();

  std::cout << "Parsed messages: " << messages.size() << std::endl;
  std::cout << "Parse errors: " << parse_errors << std::endl;
}