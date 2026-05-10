#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "../parser/parser.h"

struct ReplayPose
{

    uint64_t timestamp = 0;

    // Orientation
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    // Position
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // Velocity
    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
};

class ReplayEngine
{

public:
    // Final synchronized replay poses

    std::vector<
        ReplayPose>
        poses;

    // Build trajectory from MAVLink stream

    void build_trajectory(
        Parser &parser);

    // Synchronized playback to SITL over UDP

    void playback_sitl_udp(
        const std::string &ip,
        int port);

    // Export replay trajectory

    void export_csv(
        const std::string &filename);

    void playback_realtime();

    void clear();
};
