cmake_minimum_required(VERSION 3.10)

project(
    drone_sim_mavlink
    VERSION 1.0
    DESCRIPTION "A MAVLink telemetry replay, analysis and simulation tool"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)


# Source Files

set(VIDEO_SRC

    src/video/processor/frame_overlay.cpp
    src/video/connector/rtsp.cpp
    src/video/render/render.cpp
)

set(TELEMETRY_SRC

    src/telemetry/parser/parser.cpp
    src/telemetry/decoder/decoder.cpp
    src/telemetry/message_definition/message_definition.cpp
    src/telemetry/replay/replay.cpp
    src/telemetry/analyzer/analyzer.cpp
    src/telemetry/report/report.cpp
    src/telemetry/connector/file.cpp
    src/telemetry/connector/udp.cpp
)

set(EXTERNAL_SRC

    external/tinyxml2/tinyxml2.cpp
)


# Main Executable

add_executable(

    telemetry_app

    apps/telemetry_main.cpp

    ${TELEMETRY_SRC}
    ${EXTERNAL_SRC}
)

add_executable(

    video_app

    apps/video_main.cpp

    ${VIDEO_SRC}
)


# Include Directories

target_include_directories(

    telemetry_app

    PRIVATE

    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/external/tinyxml2
)

target_include_directories(

    video_app

    PRIVATE

    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)


target_link_libraries(

    video_app

    PRIVATE

    ${OpenCV_LIBS}
)


# MAVLink XML Path

target_compile_definitions(

    telemetry_app

    PRIVATE

    MAVLINK_XML_DIR="${PROJECT_SOURCE_DIR}/external/mavlink/message_definition/v1.0"
)