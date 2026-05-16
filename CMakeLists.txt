cmake_minimum_required(VERSION 3.10)

project(
    drone_sim_mavlink
    VERSION 1.0
    DESCRIPTION "A MAVLink telemetry replay, analysis and simulation tool"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Source Files
#

set(TELEMETRY_SRC

    src/telemetry/parser/parser.cpp
    src/telemetry/decoder/decoder.cpp
    src/telemetry/message_definition/message_definition.cpp
    src/telemetry/replay/replay.cpp
    src/telemetry/analyzer/analyzer.cpp
    src/telemetry/report/report.cpp
)

set(EXTERNAL_SRC

    external/tinyxml2/tinyxml2.cpp
)

#
# Main Executable
#

add_executable(

    replay_app

    apps/main.cpp

    ${TELEMETRY_SRC}
    ${EXTERNAL_SRC}
)

#
# Include Directories
#

target_include_directories(

    replay_app

    PRIVATE

    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/external/tinyxml2
)

#
# MAVLink XML Path
#

target_compile_definitions(

    replay_app

    PRIVATE

    MAVLINK_XML_DIR="${PROJECT_SOURCE_DIR}/external/mavlink/message_definition/v1.0"
)