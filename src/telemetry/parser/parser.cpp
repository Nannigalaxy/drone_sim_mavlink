#include "parser.h"

#include <chrono>
#include <iostream>

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

static uint64_t read_be_u64(const uint8_t *b) {
    return ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48)
           | ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32)
           | ((uint64_t)b[4] << 24) | ((uint64_t)b[5] << 16)
           | ((uint64_t)b[6] << 8) | ((uint64_t)b[7]);
}

void Parser::parse(TelemetrySource &source, MessageHandler handler) {
    mavlink_status_t status{};
    mavlink_message_t msg{};

    // stream buffer
    uint8_t buffer[4096];

    while (!source.eof()) {
        if (source.format() == StreamFormat::TLOG) {
            uint8_t tsbuf[8];
            int ts_read = source.read(tsbuf, 8);

            if (ts_read != 8) {
                break;
            }

            uint64_t timestamp_us = read_be_u64(tsbuf);

            bool packet_complete = false;

            while (!packet_complete && !source.eof()) {
                uint8_t byte;
                int bytes_read = source.read(&byte, 1);

                if (bytes_read <= 0) {
                    break;
                }

                if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status)) {
                    TelemetryMessage tm;
                    tm.timestamp_us = timestamp_us;
                    tm.message = msg;
                    handler(tm);
                    packet_complete = true;
                }

                parse_errors = status.packet_rx_drop_count;
            }
        }

        else {
            // read full raw MAVLink stream
            int bytes_read = source.read(buffer, sizeof(buffer));

            if (bytes_read <= 0) {
                continue;
            }

            for (int i = 0; i < bytes_read; ++i) {
                if (mavlink_parse_char(
                        MAVLINK_COMM_0,
                        buffer[i],
                        &msg,
                        &status
                    )) {
                    TelemetryMessage tm;

                    auto now = std::chrono::steady_clock::now();

                    tm.timestamp_us =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            now.time_since_epoch()
                        )
                            .count();

                    tm.message = msg;
                    handler(tm);
                }

                parse_errors = status.packet_rx_drop_count;
            }
        }
    }

    std::cout << "Parse errors: " << parse_errors << std::endl;
}