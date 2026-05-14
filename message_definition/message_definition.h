#pragma once

#include <tinyxml2.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

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

void printNode(std::shared_ptr<XmlNode> node, int depth = 0);