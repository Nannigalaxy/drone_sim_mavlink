#pragma once

#include <map>
#include <string>

extern "C" {
#include "../external/c_library_v2/common/mavlink.h"
}

class Decoder {
public:
  static size_t get_type_size(const std::string &type);
  static double extract_field_value(const mavlink_message_t &msg, size_t offset,
                                    const std::string &type);
};