#pragma once

#include <map>
#include <memory>
#include <string>

#include "telemetry/message_definition/message_definition.h"
#include "telemetry/parser/parser.h"

class ReplayEngine {
public:
  void load_message_definitions();

  void playback_realtime(Parser &parser);

private:
  std::map<int, MessageConfig> config;
};