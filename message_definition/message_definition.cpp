#include "message_definition.h"

#include <iostream>

using namespace tinyxml2;

std::shared_ptr<XmlNode> XmlLoader::load(const std::string &filename) {
  XMLDocument doc;

  XMLError err = doc.LoadFile(filename.c_str());

  if (err != XML_SUCCESS) {
    std::cerr << "Failed to load XML: " << filename << std::endl;

    return nullptr;
  }

  XMLElement *root = doc.RootElement();

  if (!root) {
    std::cerr << "No root element found\n";

    return nullptr;
  }

  return parseElement(root);
}

std::shared_ptr<XmlNode> XmlLoader::parseElement(XMLElement *elem) {
  auto node = std::make_shared<XmlNode>();

  node->name = elem->Name();

  // text
  if (elem->GetText()) {
    node->text = elem->GetText();
  }

  // attributes
  const XMLAttribute *attr = elem->FirstAttribute();

  while (attr) {
    node->attributes[attr->Name()] = attr->Value();

    attr = attr->Next();
  }

  // children
  XMLElement *child = elem->FirstChildElement();

  while (child) {
    node->children.push_back(parseElement(child));

    child = child->NextSiblingElement();
  }

  return node;
}

void printNode(std::shared_ptr<XmlNode> node, int depth) {
  std::string indent(depth * 2, ' ');

  std::cout << indent << "Tag: " << node->name << std::endl;

  if (!node->text.empty()) {
    std::cout << indent << "Text: " << node->text << std::endl;
  }

  for (auto &attr : node->attributes) {
    std::cout << indent << "Attr: " << attr.first << " = " << attr.second
              << std::endl;
  }

  for (auto &child : node->children) {
    printNode(child, depth + 1);
  }
}