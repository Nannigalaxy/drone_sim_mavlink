cmake_minimum_required(VERSION 3.10)

project(drone_sim_mavlink)

set(CMAKE_CXX_STANDARD 17)

add_executable(

    drone_sim_mavlink

    main.cpp

    parser/parser.cpp
    analyzer/analyzer.cpp
    report/report.cpp
    replay/replay.cpp
)

include_directories(

    external/c_library_v2
    external
)
