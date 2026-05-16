#pragma once

#include <string>

#include "../analyzer/analyzer.h"
#include "../message_definition/message_definition.h"
#include "../parser/parser.h"

class ReportGenerator {
public:
  void generate_json_report(Analyzer &analyzer, const std::string &output_file);

  void export_csv_sheet(Parser &parser, MessageRegistry &registry,
                        const std::string &filename);
};