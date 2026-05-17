#include <iostream>
#include <memory>
#include <string>

#include "telemetry/analyzer/analyzer.h"
#include "telemetry/connector/file.h"
#include "telemetry/connector/telemetry.h"
#include "telemetry/connector/udp.h"
#include "telemetry/message_definition/message_definition.h"
#include "telemetry/parser/parser.h"
#include "telemetry/replay/replay.h"
#include "telemetry/report/report.h"

static void print_usage() {
    std::cout

        << "Usage:\n\n"

        << "./telemetry_app "
        << "--analyze-summary "
        << "--file flight.tlog\n\n"

        << "./telemetry_app "
        << "--analyze-full "
        << "--file flight.tlog\n\n"

        << "./telemetry_app "
        << "--replay "
        << "--file flight.tlog\n\n"

        << "./telemetry_app "
        << "--replay "
        << "--udp 14550 [bind_ip]\n"

        << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_usage();

        return 1;
    }

    std::string mode = argv[1];

    // create source based on arguments

    std::unique_ptr<TelemetrySource> source;

    std::string source_type = argv[2];

    // File source
    if (source_type == "--file") {
        std::string file = argv[3];

        source = std::make_unique<FileTelemetrySource>(file);
    }

    // UDP source
    else if (source_type == "--udp") {
        int port = std::stoi(argv[3]);
        std::string ipaddr = "0.0.0.0";

        if (argc >= 5) {
            ipaddr = argv[4];
        }
        source = std::make_unique<UdpTelemetrySource>(ipaddr, port);
    }

    else {
        std::cout << "Unknown source type" << std::endl;
        print_usage();

        return 1;
    }

    // Open the source
    if (!source->open()) {
        std::cout << "Failed to open telemetry source" << std::endl;
        return 1;
    }

    // init components
    Analyzer analyzer;
    ReplayEngine replay;
    MessageStore store;
    Parser parser;

    std::cout << "Starting telemetry stream..." << std::endl;

    parser.parse(*source, [&](const TelemetryMessage &msg) {
        if (mode == "--analyze-agg" || mode == "--analyze-full") {
            analyzer.process_message(msg);
            store.store_message(msg);
        }

        else if (mode == "--replay") {
            replay.process_message(msg);
        }
    });

    source->close();

    // at the end of the stream, generate report or do replay
    if (mode == "--analyze-full") {
        ReportGenerator report;
        MessageRegistry registry;

        report.export_csv_sheet(store, registry, "output/full_log.csv");

        std::cout << "Full CSV report generated" << std::endl;
    }

    else if (mode == "--analyze-summary") {
        ReportGenerator report;
        report.generate_json_report(analyzer, "output/summary_report.json");

        std::cout << "Summary report generated" << std::endl;
    }

    else if (mode == "--replay") {
        std::cout << "Replay completed" << std::endl;
    }

    else {
        std::cout << "Unknown mode" << std::endl;
        print_usage();

        return 1;
    }

    return 0;
}