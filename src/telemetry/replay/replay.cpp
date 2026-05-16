#include "replay.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "telemetry/decoder/decoder.h"

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}
ReplayEngine::ReplayEngine() : first_message_(true), first_log_timestamp_(0) {}

void ReplayEngine::process_message(const TelemetryMessage &msg) {
    if (config.empty()) {
        MessageRegistry registry;
        config = registry.get_config();
    }

    if (first_message_) {
        first_message_ = false;
        first_log_timestamp_ = msg.timestamp_us;
        wall_start_ = std::chrono::steady_clock::now();
    }

    auto target =
        wall_start_
        + std::chrono::microseconds(msg.timestamp_us - first_log_timestamp_);

    std::this_thread::sleep_until(target);

    int id = msg.message.msgid;

    if (!config.count(id)) {
        return;
    }

    auto &def = config[id];

    std::cout << "\n=========================\n";
    std::cout << "MSG: " << def.field << " (" << id << ")" << std::endl;
    std::cout << "SYS: " << (int)msg.message.sysid
              << " COMP: " << (int)msg.message.compid << std::endl;
    std::cout << "TIME: " << msg.timestamp_us << std::endl;

    for (auto &field : def.sub) {
        double value = Decoder::extract_field_value(
            msg.message,
            field.offset,
            field.datatype
        );

        std::cout << "  " << field.name << " = " << value << " ["
                  << field.datatype << "]" << std::endl;
    }
}