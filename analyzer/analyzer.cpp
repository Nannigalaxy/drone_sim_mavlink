#include "analyzer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#include "../external/json.hpp"

using json = nlohmann::json;

extern "C"
{
#include "../external/c_library_v2/common/mavlink.h"
}

void update_field_stats(

    FieldStats &field, double value, const std::string &datatype)
{
    field.datatype = datatype;
    field.message_count++;

    if (std::isnan(value))
    {
        field.null_count++;

        return;
    }

    if (value < field.min)
    {
        field.min = value;
    }

    if (value > field.max)
    {
        field.max = value;
    }

    field.observed_count++;
}

void Analyzer::load_config(const std::string &filename)
{

    std::ifstream file(filename);

    if (!file.is_open())
    {

        std::cout << "Failed to open config" << std::endl;

        return;
    }

    json j;
    file >> j;

    for (auto &item : j.items())
    {
        int msg_id = std::stoi(item.key());
        MessageConfig cfg;
        cfg.field = item.value()["field"];
        cfg.sub = item.value()["sub"].get<std::vector<std::string>>();
        config[msg_id] = cfg;
    }
}

void Analyzer::initialize_registry()
{

    // HEARTBEAT
    registry["HEARTBEAT"]["type"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            return (double)hb.type;
        },

        "uint8_t"};

    registry["HEARTBEAT"]["autopilot"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            return (double)hb.autopilot;
        },

        "uint8_t"};

    registry["HEARTBEAT"]["base_mode"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            return (double)hb.base_mode;
        },

        "uint8_t"};

    registry["HEARTBEAT"]["custom_mode"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            return (double)hb.custom_mode;
        },

        "uint32_t"};

    registry["HEARTBEAT"]["system_status"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            return (double)hb.system_status;
        },

        "uint8_t"};

    // ATTITUDE

    registry["ATTITUDE"]["roll"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.roll;
        },

        "float"};

    registry["ATTITUDE"]["pitch"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.pitch;
        },

        "float"};

    registry["ATTITUDE"]["yaw"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.yaw;
        },

        "float"};

    registry["ATTITUDE"]["rollspeed"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.rollspeed;
        },

        "float"};

    registry["ATTITUDE"]["pitchspeed"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.pitchspeed;
        },

        "float"};

    registry["ATTITUDE"]["yawspeed"] = {

        [](const mavlink_message_t &msg)
        {
            mavlink_attitude_t att;

            mavlink_msg_attitude_decode(&msg, &att);

            return (double)att.yawspeed;
        },

        "float"};
}

void Analyzer::process(Parser &parser)
{

    initialize_registry();

    load_config("../config/message_types_subcat_id.json");

    for (auto &msg : parser.messages)
    {

        int id = msg.msgid;

        if (!config.count(id))
        {
            continue;
        }

        std::string msg_name = config[id].field;

        if (stats.find(id) == stats.end())
        {
            MsgStats s;
            s.msg_id = id;
            s.name = msg_name;
            stats[id] = s;
        }

        stats[id].count++;

        // protocol

        if (msg.magic == MAVLINK_STX_MAVLINK1)
        {

            protocol = "MAVLink v1";
        }
        else if (msg.magic == MAVLINK_STX)
        {

            protocol = "MAVLink v2";
        }

        // autopilot

        if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
        {

            mavlink_heartbeat_t hb;

            mavlink_msg_heartbeat_decode(&msg, &hb);

            if (hb.autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA)
            {

                autopilot = "ArduPilot";
            }
            else if (hb.autopilot == MAV_AUTOPILOT_PX4)
            {

                autopilot = "PX4";
            }
        }

        for (auto &field_name : config[id].sub)
        {

            if (!registry[msg_name].count(field_name))
            {
                continue;
            }

            auto &extractor = registry[msg_name][field_name];

            double value = extractor.extractor(msg);

            FieldStats &field = stats[id].fields[field_name];
            field.name = field_name;

            update_field_stats(field, value, extractor.datatype);
        }
    }

    std::cout << "Analysis complete" << std::endl;
}
