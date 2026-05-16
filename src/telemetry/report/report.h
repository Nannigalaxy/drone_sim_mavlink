#pragma once

#include <string>

#include "telemetry/analyzer/analyzer.h"
#include "telemetry/message_definition/message_definition.h"
#include "telemetry/parser/parser.h"

class ReportGenerator {
  public:
    void
    generate_json_report(Analyzer &analyzer, const std::string &output_file);

    void export_csv_sheet(
        MessageStore &store,
        MessageRegistry &registry,
        const std::string &filename
    );
};