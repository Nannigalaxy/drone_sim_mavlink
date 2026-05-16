#include "analyzer.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include "telemetry/decoder/decoder.h"
#include "telemetry/message_definition/message_definition.h"

extern "C" {
#include "external/c_library_v2/common/mavlink.h"
}

void update_field_stats(FieldStats &field, double value,
                        const std::string &datatype) {
  field.datatype = datatype;

  field.message_count++;

  if (std::isnan(value)) {
    field.null_count++;
    return;
  }

  if (value < field.min) {
    field.min = value;
  }

  if (value > field.max) {
    field.max = value;
  }

  field.observed_count++;
}

void Analyzer::process(Parser &parser) {
  MessageRegistry registry;

  config = registry.get_config();

  for (auto &msg : parser.messages) {
    int id = msg.message.msgid;

    if (!config.count(id)) {
      continue;
    }

    auto &cfg = config[id];

    // create stats entry
    if (!stats.count(id)) {
      MsgStats s;

      s.msg_id = id;
      s.name = cfg.field;

      stats[id] = s;
    }

    stats[id].count++;

    // MAVLink protocol version
    if (msg.message.magic == MAVLINK_STX_MAVLINK1) {
      protocol = "MAVLink v1";
    } else if (msg.message.magic == MAVLINK_STX) {
      protocol = "MAVLink v2";
    }

    // autopilot detection
    if (msg.message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
      mavlink_heartbeat_t hb;

      mavlink_msg_heartbeat_decode(&msg.message, &hb);

      if (hb.autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA) {
        autopilot = "ArduPilot";
      } else if (hb.autopilot == MAV_AUTOPILOT_PX4) {
        autopilot = "PX4";
      }
    }
    size_t current_offset = 0;
    // generic field tracking
    for (auto &field_def : cfg.sub) {
      field_def.offset = current_offset;

      current_offset += Decoder::get_type_size(field_def.datatype);

      FieldStats &field = stats[id].fields[field_def.name];

      field.name = field_def.name;
      field.datatype = field_def.datatype;

      double value = Decoder::extract_field_value(msg.message, field_def.offset,
                                                  field_def.datatype);

      update_field_stats(field, value, field_def.datatype);
    }
  }

  std::cout << "Analysis complete" << std::endl;
}