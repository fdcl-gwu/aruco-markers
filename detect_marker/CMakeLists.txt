cmake_minimum_required(VERSION 3.16.3)
project(detect_markers)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../common/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(detect_markers_src
    src/main.cpp
   )
add_executable(detect_markers ${detect_markers_src})
target_link_libraries(detect_markers
    ${OpenCV_LIBRARIES}
    )

target_compile_options(detect_markers
    PRIVATE -O3 -std=c++11
    )


