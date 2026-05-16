#include "message_definition.h"

#include <iostream>

#include "telemetry/decoder/decoder.h"

using namespace tinyxml2;

std::shared_ptr<XmlNode> XmlLoader::load(const std::string &filename) {
  XMLDocument doc;

  if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
    return nullptr;
  }

  return parseElement(doc.RootElement());
}

std::shared_ptr<XmlNode> XmlLoader::parseElement(XMLElement *elem) {
  auto node = std::make_shared<XmlNode>();

  node->name = elem->Name();

  if (elem->GetText()) {
    node->text = elem->GetText();
  }

  const XMLAttribute *attr = elem->FirstAttribute();

  while (attr) {
    node->attributes[attr->Name()] = attr->Value();

    attr = attr->Next();
  }

  XMLElement *child = elem->FirstChildElement();

  while (child) {
    node->children.push_back(parseElement(child));

    child = child->NextSiblingElement();
  }

  return node;
}

void MessageRegistry::collect_messages(std::shared_ptr<XmlNode> node) {
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
    collect_messages(child);
  }
}

bool MessageRegistry::load_from_xml(const std::string &filename) {
  XmlLoader loader;

  auto root = loader.load(filename);

  if (!root) {
    return false;
  }

  collect_messages(root);

  return true;
}

MessageRegistry::MessageRegistry() {
  std::string path = std::string(MAVLINK_XML_DIR) + "/common.xml";

  if (!load_from_xml(path)) {
    std::cerr << "Failed to load MAVLink XML: " << path << std::endl;
  } else {
    std::cout << "Loaded " << config.size() << " MAVLink definitions"
              << std::endl;
  }
}

const std::map<int, MessageConfig> &MessageRegistry::get_config() const {
  return config;
}