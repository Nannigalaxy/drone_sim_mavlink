#pragma once

#include <cstddef>
#include <cstdint>

enum class StreamFormat {
    RAW_MAVLINK,
    TLOG
};

class TelemetrySource {
  public:
    virtual ~TelemetrySource() = default;

    virtual bool open() = 0;

    virtual int read(uint8_t *buffer, size_t max_size) = 0;

    virtual bool eof() const = 0;

    virtual void close() = 0;

    virtual bool has_embedded_timestamps() const = 0;

    virtual StreamFormat format() const = 0;
};