#include "analyzer.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include "../decoder/decoder.h"
#include "../message_definition/message_definition.h"

extern "C" {
#include "../external/c_library_v2/common/mavlink.h"
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

static void collect_messages(std::shared_ptr<XmlNode> node,
                             std::map<int, MessageConfig> &config) {
  if (!node) {
    return;
  }

  // message node
  if (node->name == "message") {
    if (!node->attributes.count("id") || !node->attributes.count("name")) {
      return;
    }

    int id = std::stoi(node->attributes["id"]);

    MessageConfig msg;

    msg.field = node->attributes["name"];

    // collect fields
    for (auto &child : node->children) {
      if (child->name == "field") {
        if (child->attributes.count("name")) {
          FieldDefinition field_def;
          field_def.name = child->attributes["name"];
          field_def.datatype = child->attributes["type"];
          msg.sub.push_back(field_def);
        }
      }
    }

    config[id] = msg;
  }

  // recurse
  for (auto &child : node->children) {
    collect_messages(child, config);
  }
}

void Analyzer::load_xml_definitions() {
  XmlLoader loader;

  // TODO: support multiple XML files for different MAVLink versions
  // for now we just load the common.xml which is shared across versions
  auto root = loader.load(std::string(MAVLINK_XML_DIR) + "/common.xml");

  if (!root) {
    std::cerr << "Failed to load MAVLink XML" << std::endl;

    return;
  }

  collect_messages(root, config);

  std::cout << "Loaded " << config.size() << " MAVLink message definitions"
            << std::endl;
}

void Analyzer::process(Parser &parser) {
  load_xml_definitions();

  for (auto &msg : parser.messages) {
    int id = msg.msgid;

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
    if (msg.magic == MAVLINK_STX_MAVLINK1) {
      protocol = "MAVLink v1";
    } else if (msg.magic == MAVLINK_STX) {
      protocol = "MAVLink v2";
    }

    // autopilot detection
    if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
      mavlink_heartbeat_t hb;

      mavlink_msg_heartbeat_decode(&msg, &hb);

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

      double value = Decoder::extract_field_value(msg, field_def.offset,
                                                  field_def.datatype);

      update_field_stats(field, value, field_def.datatype);
    }
  }

  std::cout << "Analysis complete" << std::endl;
}