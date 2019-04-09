cmake_minimum_required(VERSION 2.4)
project(draw_cube)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(draw_cube_src
    src/main.cpp
   )
add_executable(draw_cube ${draw_cube_src})
target_link_libraries(draw_cube
    ${OpenCV_LIBRARIES}
    )

target_compile_options(draw_cube
    PRIVATE -O3 -std=c++11
    )


