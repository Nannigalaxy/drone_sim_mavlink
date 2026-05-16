#include "report.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "../decoder/decoder.h"
#include "../external/json.hpp"

using json = nlohmann::json;

void ReportGenerator::generate_json_report(Analyzer &analyzer,
                                           const std::string &output_file) {
  json report;

  report["protocol"] = analyzer.protocol;
  report["autopilot"] = analyzer.autopilot;

  json messages = json::array();

  for (auto &s : analyzer.stats) {
    json msg;

    msg["id"] = s.second.msg_id;
    msg["name"] = s.second.name;
    msg["count"] = s.second.count;

    json fields = json::array();

    for (auto &f : s.second.fields) {
      json field;

      field["name"] = f.second.name;
      field["datatype"] = f.second.datatype;

      if (f.second.observed_count > 0) {
        field["min"] = f.second.min;
        field["max"] = f.second.max;
      } else {
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

  if (!out.is_open()) {
    std::cout << "Failed to create report" << std::endl;

    return;
  }

  out << report.dump(4);
  out.close();

  std::cout << "JSON report generated: " << output_file << std::endl;
}

void ReportGenerator::export_csv_sheet(Parser &parser,
                                       MessageRegistry &registry,
                                       const std::string &filename) {
  std::ofstream csv(filename);

  if (!csv.is_open()) {
    std::cout << "Failed to create CSV" << std::endl;

    return;
  }

  // available columns in the format "message.field"
  std::set<std::string> used_columns;

  for (auto &tm : parser.messages) {
    auto &msg = tm.message;

    int id = msg.msgid;

    if (!registry.get_config().count(id)) {
      continue;
    }

    const auto &def = registry.get_config().at(id);

    for (const auto &field : def.sub) {
      std::string col = def.field + "." + field.name;

      used_columns.insert(col);
    }
  }

  std::vector<std::string> columns(used_columns.begin(), used_columns.end());

  // header definition
  csv << "timestamp,delta_from_previous_us,msg_ids";
  for (const auto &col : columns) {
    csv << "," << col;
  }
  csv << "\n";

  std::map<std::string, std::string> current_state;

  for (const auto &col : columns) {
    current_state[col] = "nan";
  }

  // aggregate by timestamp
  uint64_t current_timestamp = 0;
  uint64_t previous_emitted_timestamp = 0;
  bool first_row = true;
  std::set<int> timestamp_msg_ids;
  uint64_t delta_us = 0;

  for (auto &tm : parser.messages) {
    auto &msg = tm.message;

    int id = msg.msgid;

    if (!registry.get_config().count(id)) {
      continue;
    }

    uint64_t bucket_timestamp = tm.timestamp_us;

    if (first_row) {
      current_timestamp = bucket_timestamp;

      first_row = false;
    }

    if (bucket_timestamp != current_timestamp) {
      std::ostringstream msg_ids_stream;

      bool first = true;

      for (int mid : timestamp_msg_ids) {
        if (!first) {
          msg_ids_stream << "*";
        }

        msg_ids_stream << mid;

        first = false;
      }

      if (previous_emitted_timestamp != 0) {
        delta_us = current_timestamp - previous_emitted_timestamp;
      }

      csv << current_timestamp << "," << delta_us << ","
          << msg_ids_stream.str();

      for (const auto &col : columns) {
        csv << "," << current_state[col];
      }

      csv << "\n";

      // reset aggregation state
      timestamp_msg_ids.clear();

      previous_emitted_timestamp = current_timestamp;
      current_timestamp = bucket_timestamp;
    }

    if (previous_emitted_timestamp != 0) {
      delta_us = current_timestamp - previous_emitted_timestamp;
    }

    // aggregate current message
    timestamp_msg_ids.insert(msg.msgid);

    const auto &def = registry.get_config().at(id);

    for (const auto &field : def.sub) {
      double value =
          Decoder::extract_field_value(msg, field.offset, field.datatype);

      std::string key = def.field + "." + field.name;

      std::ostringstream oss;

      oss << std::setprecision(15) << value;

      current_state[key] = oss.str();
    }
  }

  csv.close();

  std::cout << "CSV report generated: " << filename << std::endl;
}