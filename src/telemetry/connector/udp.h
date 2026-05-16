#pragma once

#include <chrono>
#include <netinet/in.h>
#include <string>

#include "telemetry/connector/telemetry.h"

class UdpTelemetrySource : public TelemetrySource {
  public:
    UdpTelemetrySource(const std::string &host, int port);

    bool open() override;

    int read(uint8_t *buffer, size_t max_size) override;

    bool eof() const override;

    void close() override;

    bool has_embedded_timestamps() const override;

    StreamFormat format() const override;

  private:
    std::string host_;

    int port_;

    int socket_fd_;

    bool connected_;

    sockaddr_in peer_addr_;

    socklen_t peer_len_;

    bool connection_lost_;

    std::chrono::steady_clock::time_point last_packet_time_;
};