CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
  6502_Simulator
  VERSION 1.0.0
  DESCRIPTION "6502 Simulator"
)

LIST(APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
)

#include(${PROJECT_SOURCE_DIR}/CMake/BuildSimulator.cmake)
#include(${PROJECT_SOURCE_DIR}/CMake/BuildImGuiDemo.cmake)
include(${PROJECT_SOURCE_DIR}/CMake/BuildDemo.cmake)

