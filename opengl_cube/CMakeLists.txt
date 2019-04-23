cmake_minimum_required(VERSION 2.6)
project(opengl_cube)

set (CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
include_directories($(OPENGL_INCLUDE_DIRS))

add_subdirectory(../libraries/glfw glfw)

include_directories(../libraries/glfw/include)
include_directories(../libraries/glm)

find_package(GLEW REQUIRED)
include_directories($(GLEW_INCLUDE_DIRS))
link_libraries(${GLEW_LIBRARIES})    

SET(opengl_libs ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} glfw)
link_directories (${OPENGLEXAMPLES_BINARY_DIR}/bin)

add_executable (opengl_cube 
    src/main.cpp
    )
target_link_libraries(opengl_cube ${opengl_libs})



