#include "replay.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdint>

extern "C"
{
#include "../external/c_library_v2/common/mavlink.h"
}

void ReplayEngine::build_trajectory(
    Parser &parser)
{
    mavlink_attitude_t latest_att{};
    mavlink_local_position_ned_t latest_pos{};

    bool have_attitude = false;
    bool have_position = false;

    for (auto &msg : parser.messages)
    {
        if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE)
        {
            mavlink_msg_attitude_decode(&msg, &latest_att);
            have_attitude = true;
        }
        else if (msg.msgid == MAVLINK_MSG_ID_LOCAL_POSITION_NED)
        {
            mavlink_msg_local_position_ned_decode(&msg, &latest_pos);
            have_position = true;
        }

        if (have_attitude && have_position)
        {
            uint64_t att_ts = latest_att.time_boot_ms;
            uint64_t pos_ts = latest_pos.time_boot_ms;

            uint64_t delta = (att_ts > pos_ts)
                                 ? (att_ts - pos_ts)
                                 : (pos_ts - att_ts);

            if (delta < 100)
            {
                ReplayPose pose;
                pose.timestamp = std::max(att_ts, pos_ts);

                pose.roll = latest_att.roll;
                pose.pitch = latest_att.pitch;
                pose.yaw = latest_att.yaw;

                pose.x = latest_pos.x;
                pose.y = latest_pos.y;
                pose.z = latest_pos.z;
                pose.vx = latest_pos.vx;
                pose.vy = latest_pos.vy;
                pose.vz = latest_pos.vz;

                poses.push_back(pose);

                have_attitude = false;
                have_position = false;
            }
        }
    }

    std::cout << "Replay poses built: " << poses.size() << std::endl;
}

void ReplayEngine::export_csv(const std::string &filename)
{
    std::ofstream csv(filename);
    if (!csv.is_open())
    {
        std::cout << "Failed to create CSV" << std::endl;
        return;
    }

    csv << "timestamp,"
        << "x,y,z,"
        << "roll,pitch,yaw,"
        << "vx,vy,vz\n";

    for (auto &p : poses)
    {
        csv << p.timestamp << ","
            << p.x << "," << p.y << "," << p.z << ","
            << p.roll << "," << p.pitch << "," << p.yaw << ","
            << p.vx << "," << p.vy << "," << p.vz
            << "\n";
    }

    csv.close();
    std::cout << "Replay CSV exported: " << filename << std::endl;
}

static void send_mavlink(
    int sock,
    const sockaddr_in &addr,
    mavlink_message_t &msg)
{
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0,
           reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
}

static void heartbeat_thread(
    int sock,
    const sockaddr_in &addr,
    std::atomic<bool> &running)
{
    while (running.load())
    {
        mavlink_message_t hb_msg;
        mavlink_msg_heartbeat_pack(
            255, // sysid
            190, // compid
            &hb_msg,
            MAV_TYPE_GCS,
            MAV_AUTOPILOT_INVALID,
            0,
            0,
            MAV_STATE_ACTIVE);

        send_mavlink(sock, addr, hb_msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static bool wait_for_ack(
    int sock,
    uint16_t expected_cmd,
    int timeout_ms = 5000)
{
    using clock = std::chrono::steady_clock;
    auto deadline = clock::now() + std::chrono::milliseconds(timeout_ms);

    while (clock::now() < deadline)
    {
        uint8_t rx[2048];
        sockaddr_in src{};
        socklen_t addrlen = sizeof(src);

        int n = recvfrom(sock, rx, sizeof(rx), 0,
                         reinterpret_cast<sockaddr *>(&src), &addrlen);
        if (n > 0)
        {
            mavlink_message_t rx_msg;
            mavlink_status_t status;

            for (int i = 0; i < n; ++i)
            {
                if (mavlink_parse_char(MAVLINK_COMM_1, rx[i], &rx_msg, &status))
                {
                    if (rx_msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK)
                    {
                        mavlink_command_ack_t ack;
                        mavlink_msg_command_ack_decode(&rx_msg, &ack);

                        std::cout << "COMMAND_ACK cmd=" << ack.command
                                  << " result=" << (int)ack.result << std::endl;

                        if (ack.command == expected_cmd &&
                            ack.result == MAV_RESULT_ACCEPTED)
                            return true;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cerr << "Timed out waiting for ACK of cmd=" << expected_cmd << std::endl;
    return false;
}

static bool arm_and_set_guided(
    int sock,
    const sockaddr_in &addr)
{
    mavlink_message_t msg;

    std::cout << "Setting GUIDED mode..." << std::endl;
    mavlink_msg_command_long_pack(
        255, 190, &msg,
        1, 1, // target sysid / compid
        MAV_CMD_DO_SET_MODE,
        0, // confirmation
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        4, // custom_mode: GUIDED = 4
        0, 0, 0, 0, 0);
    send_mavlink(sock, addr, msg);

    if (!wait_for_ack(sock, MAV_CMD_DO_SET_MODE, 5000))
    {
        std::cerr << "Failed to set GUIDED mode" << std::endl;
        return false;
    }

    std::cout << "Arming..." << std::endl;
    mavlink_msg_command_long_pack(
        255, 190, &msg,
        1, 1,
        MAV_CMD_COMPONENT_ARM_DISARM,
        0,
        1, // 1 = arm
        0, 0, 0, 0, 0, 0);
    send_mavlink(sock, addr, msg);

    if (!wait_for_ack(sock, MAV_CMD_COMPONENT_ARM_DISARM, 5000))
    {
        std::cerr << "Failed to arm" << std::endl;
        return false;
    }

    std::cout << "Takeoff..." << std::endl;
    mavlink_msg_command_long_pack(
        255, 190, &msg,
        1, 1,
        MAV_CMD_NAV_TAKEOFF,
        0,
        0, 0, 0, 0, // params 1-4 unused
        0, 0,       // lat/lon (use current)
        5.0f);      // altitude metres
    send_mavlink(sock, addr, msg);

    if (!wait_for_ack(sock, MAV_CMD_NAV_TAKEOFF, 5000))
    {
        std::cerr << "Failed to takeoff" << std::endl;
        return false;
    }

    std::cout << "Climbing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return true;
}

void ReplayEngine::playback_sitl_udp(
    const std::string &ip,
    int port)
{
    if (poses.empty())
    {
        std::cout << "No replay poses" << std::endl;
        return;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(15551);

    if (bind(sock,
             reinterpret_cast<sockaddr *>(&local_addr),
             sizeof(local_addr)) < 0)
    {
        std::cerr << "Bind failed on port 14551 — is QGC already running?" << std::endl;
        close(sock);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::cout << "Connecting to SITL: " << ip << ":" << port << std::endl;

    std::atomic<bool> running{true};
    std::thread hb_thread(heartbeat_thread, sock, std::cref(addr), std::ref(running));

    if (!arm_and_set_guided(sock, addr))
    {
        running.store(false);
        hb_thread.join();
        close(sock);
        return;
    }

    using clock = std::chrono::steady_clock;
    auto wall_start = clock::now();
    uint64_t log_start = poses[0].timestamp;

    for (auto &p : poses)
    {
        auto target = wall_start +
                      std::chrono::milliseconds(p.timestamp - log_start);
        std::this_thread::sleep_until(target);

        mavlink_message_t msg;

        const uint16_t TYPE_MASK = 0b0000110111000000;

        mavlink_msg_set_position_target_local_ned_pack(
            255, 190, // sender sysid / compid (GCS)
            &msg,
            static_cast<uint32_t>(p.timestamp), // time_boot_ms

            1, 1, // target sysid / compid (vehicle)

            MAV_FRAME_LOCAL_NED,

            TYPE_MASK,
            p.x, p.y, p.z,    // position   [m]
            p.vx, p.vy, p.vz, // velocity   [m/s]
            0.0f, 0.0f, 0.0f, // accel      [m/s]
            p.yaw,            // yaw        [rad]
            0.0f);            // yaw_rate   [rad/s]

        send_mavlink(sock, addr, msg);

        uint8_t rx_buffer[2048];
        sockaddr_in src_addr{};
        socklen_t addrlen = sizeof(src_addr);

        int rx_len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0,
                              reinterpret_cast<sockaddr *>(&src_addr),
                              &addrlen);

        if (rx_len > 0)
        {
            mavlink_message_t rx_msg;
            mavlink_status_t status;

            for (int i = 0; i < rx_len; ++i)
            {
                if (mavlink_parse_char(MAVLINK_COMM_1, rx_buffer[i],
                                       &rx_msg, &status))
                {
                    if (rx_msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
                        std::cout << "HEARTBEAT received from SITL" << std::endl;

                    if (rx_msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK)
                    {
                        mavlink_command_ack_t ack;
                        mavlink_msg_command_ack_decode(&rx_msg, &ack);
                        std::cout << "COMMAND_ACK cmd=" << ack.command
                                  << " result=" << (int)ack.result << std::endl;
                    }
                }
            }
        }

        std::cout << "Sent pose t=" << p.timestamp
                  << " pos=(" << p.x << ", " << p.y << ", " << p.z << ")"
                  << " yaw=" << p.yaw
                  << std::endl;
    }

    running.store(false);
    hb_thread.join();
    close(sock);

    std::cout << "Playback complete." << std::endl;
}

void ReplayEngine::playback_realtime()
{
    if (poses.empty())
    {
        std::cout << "No replay poses" << std::endl;
        return;
    }

    using clock = std::chrono::steady_clock;
    auto wall_start = clock::now();
    uint64_t log_start = poses[0].timestamp;

    for (auto &p : poses)
    {
        auto target = wall_start +
                      std::chrono::milliseconds(p.timestamp - log_start);
        std::this_thread::sleep_until(target);

        std::cout << "[" << p.timestamp << "] "
                  << "POS(" << p.x << ", " << p.y << ", " << p.z << ") "
                  << "RPY(" << p.roll << ", " << p.pitch << ", " << p.yaw << ")"
                  << std::endl;
    }
}