#pragma once

#include <fstream>
#include <string>

#include "telemetry/connector/telemetry.h"

class FileTelemetrySource : public TelemetrySource {
  public:
    explicit FileTelemetrySource(const std::string &path);

    bool open() override;

    int read(uint8_t *buffer, size_t max_size) override;

    bool eof() const override;

    void close() override;

    bool has_embedded_timestamps() const override;

    StreamFormat format() const override;

  private:
    std::string path_;

    std::ifstream file_;
};