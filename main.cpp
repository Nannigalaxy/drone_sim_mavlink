#include <iostream>
#include <string>

#include "analyzer/analyzer.h"
#include "message_definition/message_definition.h"
#include "parser/parser.h"
#include "replay/replay.h"
#include "report/report.h"

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cout << "Usage:\n\n"

              << "./drone_sim_mavlink "
              << "--analyze "
              << "[--agg | --full] "
              << "flight.tlog\n\n"

              << "./drone_sim_mavlink "
              << "--replay "
              << "flight.tlog"

              << std::endl;

    return 1;
  }

  std::string mode = argv[1];

  Parser parser;

  if (mode == "--analyze") {
    std::string report_mode = argv[2];
    std::string file = argv[3];

    std::cout << "Parsing telemetry..." << std::endl;
    parser.parse_file(file);

    std::cout << "Total messages parsed: " << parser.messages.size()
              << std::endl;

    Analyzer analyzer;
    analyzer.process(parser);

    ReportGenerator report;

    if (report_mode == "--agg") {
      report.generate_json_report(analyzer, "report.json");
      std::cout << "Aggregated report generated" << std::endl;
    }

    else if (report_mode == "--full") {
      MessageRegistry registry;
      report.export_csv_sheet(parser, registry, "full_report.csv");

      std::cout << "Full CSV report generated" << std::endl;
    }

    else {
      std::cout << "Unknown report mode" << std::endl;
      return 1;
    }

    std::cout << "Analysis completed" << std::endl;
  }

  else if (mode == "--replay") {
    std::string file = argv[2];

    std::cout << "Parsing telemetry..." << std::endl;
    parser.parse_file(file);

    std::cout << "Total messages parsed: " << parser.messages.size()
              << std::endl;

    ReplayEngine replay;
    replay.playback_realtime(parser);
  }

  else {
    std::cout << "Unknown mode" << std::endl;
    return 1;
  }

  return 0;
}