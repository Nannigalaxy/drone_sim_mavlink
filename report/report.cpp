#include "report.h"

#include <fstream>
#include <iostream>

#include "../external/json.hpp"

using json = nlohmann::json;

void ReportGenerator::generate_json_report(

    Analyzer &analyzer,
    const std::string &output_file)
{

    json report;
    report["protocol"] = analyzer.protocol;
    report["autopilot"] = analyzer.autopilot;

    json messages = json::array();

    for (auto &s : analyzer.stats)
    {

        json msg;
        msg["id"] = s.second.msg_id;
        msg["name"] = s.second.name;
        msg["count"] = s.second.count;

        json fields = json::array();

        for (auto &f : s.second.fields)
        {

            json field;

            field["name"] = f.second.name;
            field["datatype"] = f.second.datatype;
            if (f.second.observed_count > 0)
            {
                field["min"] = f.second.min;
                field["max"] = f.second.max;
            }
            else
            {
                field["min"] = nullptr;
                field["max"] = nullptr;
            }

            field["null_count"] = f.second.null_count;
            field["observed_count"] = f.second.observed_count;
            field["message_count"] = f.second.message_count;
            fields.push_back(field);
        }

        msg["fields"] = fields;
        messages.push_back(msg);
    }

    report["messages"] = messages;

    std::ofstream out(output_file);

    if (!out.is_open())
    {
        std::cout << "Failed to create report" << std::endl;

        return;
    }

    out << report.dump(4);
    out.close();

    std::cout << "Report generated: " << output_file << std::endl;
}
