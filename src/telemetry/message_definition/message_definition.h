#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <tinyxml2.h>

const std::string MESSAGE_DEFINITION_XML = "all.xml";

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
    std::set<std::string> loaded_files;

    void process_includes(
        std::shared_ptr<XmlNode> node,
        const std::filesystem::path &base_dir
    );

  private:
    std::map<int, MessageConfig> config;

    std::string xml_path =
        std::string(MAVLINK_XML_DIR) + "/" + MESSAGE_DEFINITION_XML;

    void collect_messages(std::shared_ptr<XmlNode> node);
};