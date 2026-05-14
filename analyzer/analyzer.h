
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "../parser/parser.h"

struct FieldStats
{
    std::string name;
    std::string datatype;
    double min = 1e18;
    double max = -1e18;
    int null_count = 0;
    int observed_count = 0;
    int message_count = 0;
};

struct MsgStats
{
    int msg_id = 0;
    std::string name;
    int count = 0;
    std::map<std::string, FieldStats> fields;
};

struct MessageConfig
{
    std::string field;
    std::vector<std::string> sub;
};

struct FieldExtractor
{
    std::function<double(const mavlink_message_t &)> extractor;
    std::string datatype;
};

class Analyzer
{

public:
    std::map<int, MsgStats> stats;
    std::string protocol = "Unknown";
    std::string autopilot = "Unknown";
    std::map<int, MessageConfig> config;
    std::map<std::string,

             std::map<std::string, FieldExtractor>

             >
        registry;

    void process(Parser &parser);
    void load_xml_definitions();
};
