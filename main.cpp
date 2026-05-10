#include <iostream>

#include "analyzer/analyzer.h"
#include "parser/parser.h"
#include "replay/replay.h"
#include "report/report.h"

int main(int argc, char *argv[]) {

  if (argc < 3) {

    std::cout

        << "Usage:\n"

        << "./drone_adapter "
        << "--analyze "
        << "flight.tlog"

        << std::endl;

    return 1;
  }

  std::string mode = argv[1];
  std::string file = argv[2];

  Parser parser;

  std::cout << "Parsing telemetry..." << std::endl;

  parser.parse_file(file);

  std::cout << "Total messages parsed: " << parser.messages.size() << std::endl;

  // ANALYZE

  if (mode == "--analyze") {
    Analyzer analyzer;

    analyzer.process(parser);

    ReportGenerator report;

    report.generate_json_report(analyzer, "report.json");

    ReplayEngine replay;

    replay.build_trajectory(parser);

    replay.export_csv("replay.csv");

    std::cout << "Analysis completed" << std::endl;
  }

  // REPLAY

  else if (mode == "--replay") {
    ReplayEngine replay;

    replay.build_trajectory(parser);

    replay.playback_sitl_udp("127.0.0.1", 14550);
  }

  else {
    std::cout << "Unknown mode" << std::endl;
  }

  return 0;
}
