#pragma once

#include <string>
#include <vector>

extern "C"
{
#include "../external/c_library_v2/common/mavlink.h"
}

class Parser
{

public:
    std::vector<mavlink_message_t> messages;
    size_t parse_errors = 0;

    void parse_file(const std::string &filename);
};
