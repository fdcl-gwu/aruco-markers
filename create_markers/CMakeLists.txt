cmake_minimum_required(VERSION 3.16.3)
project(aruco_markers)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)


set(generate_marker_src
    src/create_marker.cpp
   )
add_executable(generate_marker ${generate_marker_src})
target_link_libraries(generate_marker 
    ${OpenCV_LIBRARIES}
    )

target_compile_options(generate_marker
    PRIVATE -O3 -std=c++11
    )


set(generate_board_src
    src/create_board.cpp
   )
add_executable(generate_board ${generate_board_src})
target_link_libraries(generate_board 
    ${OpenCV_LIBRARIES}
    )

target_compile_options(generate_board
    PRIVATE -O3 -std=c++11
    )
