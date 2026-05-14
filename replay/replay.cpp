#include "replay.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "../decoder/decoder.h"

extern "C" {
#include "../external/c_library_v2/common/mavlink.h"
}

static void collect_messages(std::shared_ptr<XmlNode> node,
                             std::map<int, MessageConfig> &config) {
  if (!node) {
    return;
  }

  if (node->name == "message") {
    if (!node->attributes.count("id") || !node->attributes.count("name")) {
      return;
    }

    int id = std::stoi(node->attributes["id"]);

    MessageConfig msg;

    msg.field = node->attributes["name"];

    size_t current_offset = 0;

    for (auto &child : node->children) {
      if (child->name != "field") {
        continue;
      }

      FieldDefinition field;

      field.name = child->attributes["name"];

      field.datatype = child->attributes["type"];

      field.offset = current_offset;

      current_offset += Decoder::get_type_size(field.datatype);

      msg.sub.push_back(field);
    }

    config[id] = msg;
  }

  for (auto &child : node->children) {
    collect_messages(child, config);
  }
}

void ReplayEngine::load_message_definitions() {
  XmlLoader loader;

  auto root = loader.load(std::string(MAVLINK_XML_DIR) + "/common.xml");

  if (!root) {
    std::cerr << "Failed to load XML" << std::endl;

    return;
  }

  collect_messages(root, config);

  std::cout << "Loaded " << config.size() << " message definitions"
            << std::endl;
}

void ReplayEngine::playback_realtime(Parser &parser) {
  load_message_definitions();

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