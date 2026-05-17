#include "message_definition.h"

#include <filesystem>
#include <iostream>
#include <set>

#include "telemetry/decoder/decoder.h"

using namespace tinyxml2;

namespace fs = std::filesystem;

std::shared_ptr<XmlNode> XmlLoader::load(const std::string &filename) {
    XMLDocument doc;

    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to load XML: " << filename << std::endl;
        return nullptr;
    }

    return parseElement(doc.RootElement());
}

std::shared_ptr<XmlNode> XmlLoader::parseElement(XMLElement *elem) {
    if (!elem) {
        return nullptr;
    }

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
        auto parsed_child = parseElement(child);

        if (parsed_child) {
            node->children.push_back(parsed_child);
        }

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

            if (!child->attributes.count("name")
                || !child->attributes.count("type")) {
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

void MessageRegistry::process_includes(
    std::shared_ptr<XmlNode> node,
    const fs::path &base_dir
) {
    if (!node) {
        return;
    }

    for (auto &child : node->children) {
        if (child->name == "include") {
            std::string include_name = child->text;

            if (include_name.empty()) {
                continue;
            }

            fs::path include_path = base_dir / include_name;

            std::string normalized =
                fs::weakly_canonical(include_path).string();

            if (loaded_files.count(normalized)) {
                continue;
            }

            loaded_files.insert(normalized);

            std::cout << "Loading include: " << normalized << std::endl;

            XmlLoader loader;
            auto included_root = loader.load(normalized);

            if (!included_root) {
                std::cerr << "Failed include: " << normalized << std::endl;
                continue;
            }

            process_includes(included_root, include_path.parent_path());
            collect_messages(included_root);
        }

        process_includes(child, base_dir);
    }
}

bool MessageRegistry::load_from_xml(const std::string &filename) {
    fs::path xml_file = fs::absolute(filename);
    std::string normalized = fs::weakly_canonical(xml_file).string();

    if (loaded_files.count(normalized)) {
        return true;
    }

    loaded_files.insert(normalized);

    XmlLoader loader;
    auto root = loader.load(normalized);

    if (!root) {
        return false;
    }

    process_includes(root, xml_file.parent_path());
    collect_messages(root);

    return true;
}

MessageRegistry::MessageRegistry() {
    if (!load_from_xml(xml_path)) {
        std::cerr << "Failed to load MAVLink XML: " << xml_path << std::endl;

    } else {
        std::cout << "Loaded " << config.size() << " MAVLink definitions"
                  << std::endl;
    }
}

const std::map<int, MessageConfig> &MessageRegistry::get_config() const {
    return config;
}