#pragma once

#include <functional>

#include "telemetry/connector/telemetry.h"

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

struct TelemetryMessage {
    uint64_t timestamp_us = 0;

    mavlink_message_t message;
};

class Parser {
  public:
    using MessageHandler = std::function<void(const TelemetryMessage &)>;
    size_t parse_errors = 0;

    void parse(TelemetrySource &source, MessageHandler handler);
};
