cmake_minimum_required(VERSION 3.10)

project(drone_sim_mavlink)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable

add_executable(

    drone_sim_mavlink

    main.cpp

    decoder/decoder.cpp
    parser/parser.cpp
    analyzer/analyzer.cpp
    report/report.cpp
    replay/replay.cpp

    message_definition/message_definition.cpp

    external/tinyxml2/tinyxml2.cpp
)

# Include Directories

target_include_directories(
    drone_sim_mavlink
    PRIVATE

    ${PROJECT_SOURCE_DIR}

    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/external/tinyxml2

    ${PROJECT_SOURCE_DIR}/decoder
    ${PROJECT_SOURCE_DIR}/parser
    ${PROJECT_SOURCE_DIR}/analyzer
    ${PROJECT_SOURCE_DIR}/report
    ${PROJECT_SOURCE_DIR}/replay
    ${PROJECT_SOURCE_DIR}/message_definition
)

# XML Runtime Path

target_compile_definitions(
    drone_sim_mavlink
    PRIVATE
    MAVLINK_XML_DIR="${PROJECT_SOURCE_DIR}/external/mavlink/message_definition/v1.0"
)