#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "telemetry/message_definition/message_definition.h"
#include "telemetry/parser/parser.h"

class ReplayEngine {
  public:
    ReplayEngine();
    void load_message_definitions();

    void playback_realtime(Parser &parser);
    void process_message(const TelemetryMessage &msg);

  private:
    bool first_message_ = true;

    uint64_t first_log_timestamp_ = 0;

    std::chrono::steady_clock::time_point wall_start_;
    std::map<int, MessageConfig> config;
};