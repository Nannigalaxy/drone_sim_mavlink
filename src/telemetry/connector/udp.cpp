#include "telemetry/connector/udp.h"

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

StreamFormat UdpTelemetrySource::format() const {
    return StreamFormat::RAW_MAVLINK;
}

UdpTelemetrySource::UdpTelemetrySource(const std::string &host, int port)
    : host_(host), port_(port), socket_fd_(-1), connected_(false),
      peer_len_(sizeof(peer_addr_)) {}

bool UdpTelemetrySource::open() {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd_ < 0) {
        std::cerr << "Failed to create UDP socket" << std::endl;

        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Invalid bind address" << std::endl;

        return false;
    }

    if (bind(socket_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))
        < 0) {
        std::cerr << "UDP bind failed" << std::endl;

        return false;
    }

    std::cout << "Waiting for telemetry connection on " << host_ << ":" << port_
              << std::endl;

    // Wait for first packet to identify peer
    timeval tv{};
    // Set a timeout for the initial handshake
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t temp[2048];
    int bytes = recvfrom(
        socket_fd_,
        temp,
        sizeof(temp),
        0,
        reinterpret_cast<sockaddr *>(&peer_addr_),
        &peer_len_
    );

    if (bytes <= 0) {
        std::cerr << "Failed to receive handshake" << std::endl;

        return false;
    }

    // Lock socket to peer
    if (connect(
            socket_fd_,
            reinterpret_cast<sockaddr *>(&peer_addr_),
            peer_len_
        )
        < 0) {
        std::cerr << "UDP connect() failed" << std::endl;
        return false;
    }

    connected_ = true;
    char peer_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer_addr_.sin_addr, peer_ip, sizeof(peer_ip));

    std::cout << "Telemetry connected: " << peer_ip << ":"
              << ntohs(peer_addr_.sin_port) << std::endl;

    return true;
}

int UdpTelemetrySource::read(uint8_t *buffer, size_t max_size) {
    int n = recv(socket_fd_, buffer, max_size, 0);

    if (n > 0) {
        last_packet_time_ = std::chrono::steady_clock::now();
        connected_ = true;

        return n;
    }
    connected_ = false;

    return -1;
}

bool UdpTelemetrySource::eof() const {
    if (!connected_) {
        std::cout << "Connection lost. end of stream." << std::endl;
        return true;
    }

    return false;
}

void UdpTelemetrySource::close() {
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool UdpTelemetrySource::has_embedded_timestamps() const { return false; }