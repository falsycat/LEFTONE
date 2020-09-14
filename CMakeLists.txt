cmake_minimum_required(VERSION 3.16)

project(leftone C)

if (BUILD_TESTING)
  enable_testing()
endif()

set(LEFTONE_TOOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tool")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers")
set(CMAKE_C_FLAGS_RELEASE
    "${CMAKE_C_FLAGS_RELEASE} -Wno-unused-parameter")

if (WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mconsole")
endif()

find_package(Freetype REQUIRED)
find_package(GLEW     REQUIRED)
find_package(OpenGL   REQUIRED)
find_package(SDL2     REQUIRED)
find_package(msgpack  REQUIRED)

include_directories(SYSTEM
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
)
include_directories(.)

include(cmake/anysrc.cmake)
include(cmake/sos.cmake)

add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(thirdparty)
add_subdirectory(tool)
add_subdirectory(util)
