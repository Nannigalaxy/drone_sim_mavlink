#include "replay.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "telemetry/decoder/decoder.h"

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

void ReplayEngine::playback_realtime(Parser &parser) {
  MessageRegistry registry;

  config = registry.get_config();

  if (parser.messages.empty()) {
    std::cout << "No MAVLink messages" << std::endl;

    return;
  }

  using clock = std::chrono::steady_clock;

  auto wall_start = clock::now();

  uint64_t log_start = parser.messages[0].timestamp_us;

  for (auto &msg : parser.messages) {
    auto target =
        wall_start + std::chrono::microseconds(msg.timestamp_us - log_start);

    std::this_thread::sleep_until(target);

    int id = msg.message.msgid;

    if (!config.count(id)) {
      continue;
    }

    auto &def = config[id];

    std::cout << "\n=========================\n";

    std::cout << "MSG: " << def.field << " (" << id << ")" << std::endl;

    std::cout << "SYS: " << (int)msg.message.sysid
              << " COMP: " << (int)msg.message.compid << std::endl;

    std::cout << "TIME: " << msg.timestamp_us << std::endl;

    for (auto &field : def.sub) {
      double value = Decoder::extract_field_value(msg.message, field.offset,
                                                  field.datatype);

      std::cout << "  " << field.name << " = " << value << " ["
                << field.datatype << "]" << std::endl;
    }
  }

  std::cout << "\nReplay complete." << std::endl;
}