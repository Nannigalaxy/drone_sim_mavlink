#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <tinyxml2.h>

struct FieldDefinition {
    std::string name;
    std::string datatype;

    size_t offset = 0;
};

struct MessageConfig {
    std::string field;

    std::vector<FieldDefinition> sub;
};

struct XmlNode {
    std::string name;
    std::string text;

    std::map<std::string, std::string> attributes;

    std::vector<std::shared_ptr<XmlNode>> children;
};

class XmlLoader {
  public:
    std::shared_ptr<XmlNode> load(const std::string &filename);

  private:
    std::shared_ptr<XmlNode> parseElement(tinyxml2::XMLElement *elem);
};

class MessageRegistry {
  public:
    bool load_from_xml(const std::string &filename);
    MessageRegistry();
    const std::map<int, MessageConfig> &get_config() const;

  private:
    std::map<int, MessageConfig> config;

    void collect_messages(std::shared_ptr<XmlNode> node);
};