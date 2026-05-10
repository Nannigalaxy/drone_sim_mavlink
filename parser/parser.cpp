#include "parser.h"

#include <fstream>
#include <iostream>

extern "C"
{
#include "../external/c_library_v2/common/mavlink.h"
}

void Parser::parse_file(const std::string &filename)
{

    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open file" << std::endl;

        return;
    }

    mavlink_message_t msg;
    mavlink_status_t status;
    uint8_t byte;

    while (file.read((char *)&byte, 1))
    {

        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status))
        {
            messages.push_back(msg);
        }

        parse_errors = status.packet_rx_drop_count;
    }

    file.close();

    std::cout << "Parsed messages: " << messages.size() << std::endl;

    std::cout << "Parse errors: " << parse_errors << std::endl;
}
