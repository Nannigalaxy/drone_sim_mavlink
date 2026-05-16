#include "decoder.h"

#include <cmath>
#include <fstream>
#include <iostream>

size_t Decoder::get_type_size(const std::string &type) {
    static std::map<std::string, size_t> sizes = {
        {"char", 1},

        {"int8_t", 1},
        {"uint8_t", 1},

        {"int16_t", 2},
        {"uint16_t", 2},

        {"int32_t", 4},
        {"uint32_t", 4},

        {"int64_t", 8},
        {"uint64_t", 8},

        {"float", 4},
        {"double", 8}};

    if (sizes.count(type)) {
        return sizes[type];
    }

    return 0;
}

double Decoder::extract_field_value(
    const mavlink_message_t &msg,
    size_t offset,
    const std::string &type
) {
    const uint8_t *payload = reinterpret_cast<const uint8_t *>(msg.payload64);

    if (type == "float") {
        return *(float *)(payload + offset);
    }

    if (type == "double") {
        return *(double *)(payload + offset);
    }

    if (type == "int8_t") {
        return *(int8_t *)(payload + offset);
    }

    if (type == "uint8_t") {
        return *(uint8_t *)(payload + offset);
    }

    if (type == "int16_t") {
        return *(int16_t *)(payload + offset);
    }

    if (type == "uint16_t") {
        return *(uint16_t *)(payload + offset);
    }

    if (type == "int32_t") {
        return *(int32_t *)(payload + offset);
    }

    if (type == "uint32_t") {
        return *(uint32_t *)(payload + offset);
    }

    if (type == "int64_t") {
        return *(int64_t *)(payload + offset);
    }

    if (type == "uint64_t") {
        return *(uint64_t *)(payload + offset);
    }

    return NAN;
}