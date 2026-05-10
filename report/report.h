#pragma once

#include <string>

#include "../analyzer/analyzer.h"

class ReportGenerator
{

public:
    void generate_json_report(

        Analyzer &analyzer,
        const std::string &output_file);
};