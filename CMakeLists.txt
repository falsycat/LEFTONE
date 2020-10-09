cmake_minimum_required(VERSION 3.16)

project(leftone C)

include(CTest)

set(LEFTONE_C_FLAGS
  -Werror -Wall -Wextra -Wno-missing-field-initializers
  $<$<CONFIG:Release>:-Wno-unused-parameter>
  $<$<PLATFORM_ID:Windows>:-mconsole>
)
set(LEFTONE_TOOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tool")

find_package(Freetype REQUIRED)
find_package(GLEW     REQUIRED)
find_package(OpenGL   REQUIRED)
find_package(SDL2     REQUIRED)
find_package(msgpack  REQUIRED)

set(SDL2_TARGET SDL2::SDL2)
if (NOT TARGET SDL2::SDL2)
  set(SDL2_TARGET ${SDL2_LIBRARIES})
endif()

include_directories(SYSTEM
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIRS}
)
include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

include(cmake/anysrc.cmake)
include(cmake/benum.cmake)
include(cmake/crial.cmake)
include(cmake/sos.cmake)

add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(thirdparty)
add_subdirectory(tool)
add_subdirectory(util)
