cmake_minimum_required(VERSION 3.10)
project(POLYCODEGEN)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(ISL REQUIRED)

# print debug messages
message(STATUS "Found ISL ${ISL_VERSION}")
message(STATUS "Using ISL")

# include the necessary directories
include_directories(${ISL_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable(codegen ${SOURCES})

target_link_libraries(codegen
  PRIVATE
  ${ISL_LIBRARY}
)

target_compile_options(codegen PRIVATE -fno-rtti)
