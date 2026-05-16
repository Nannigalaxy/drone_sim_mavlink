#include "telemetry/connector/file.h"

StreamFormat FileTelemetrySource::format() const {
    return StreamFormat::RAW_MAVLINK;
}

FileTelemetrySource::FileTelemetrySource(const std::string &path)
    : path_(path) {}

bool FileTelemetrySource::open() {
    file_.open(path_, std::ios::binary);

    return file_.is_open();
}

int FileTelemetrySource::read(uint8_t *buffer, size_t max_size) {
    if (!file_.is_open()) {
        return -1;
    }

    file_.read(reinterpret_cast<char *>(buffer), max_size);

    return static_cast<int>(file_.gcount());
}

bool FileTelemetrySource::eof() const { return file_.eof(); }

void FileTelemetrySource::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

bool FileTelemetrySource::has_embedded_timestamps() const { return false; }