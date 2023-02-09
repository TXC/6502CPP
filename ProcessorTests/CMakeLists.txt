CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
  6502_Processor_Tests
  VERSION 1.0.0
  DESCRIPTION "6502 Processor Tests"
  LANGUAGES CXX
)

SET(APP_NAME ProcessorTests)

INCLUDE(Common)

ADD_EXECUTABLE(${APP_NAME})

#COMMON_LANGUAGE_PARAMETERS()
COMMON_SET_PROJECT_FLAGS()

set_compiler_version(
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS ON
  TARGET "${APP_NAME}"
)

TARGET_SOURCES(${APP_NAME}
  PUBLIC  MainTest.hpp
          ProcessorTests.hpp
          FunctionalProcessorTests.hpp

  PRIVATE MainTest.cpp
          ProcessorTests.cpp
          FunctionalProcessorTests.cpp
)

OPTION(ENABLE_TEST "Enable Tests" ON)

SET(TestDataDirectory "${CMAKE_CURRENT_SOURCE_DIR}/FunctionalTests/" CACHE PATH "Path to directory where binary test files can be found")

IF(DEFINED TestDataDirectory AND IS_DIRECTORY ${TestDataDirectory})
  TARGET_COMPILE_DEFINITIONS(${APP_NAME} PRIVATE TESTDIR=${TestDataDirectory})
ELSEIF(DEFINED TestDataDirectory AND NOT IS_DIRECTORY ${TestDataDirectory})
  MESSAGE(SEND_ERROR "[ TESTDIR ] - Path not found - ${TestDataDirectory}")
ENDIF()

IF(TARGET Processor)
  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE Processor)
ELSE()
  FIND_PACKAGE(Processor REQUIRED)
  IF(Processor_FOUND)
    TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE Processor::Processor)
  ENDIF()
ENDIF()

FIND_PACKAGE(fmt REQUIRED CONFIG)
IF(fmt_FOUND)
  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE fmt::fmt-header-only)
ENDIF()

#FIND_PACKAGE(spdlog REQUIRED)
#IF(spdlog_FOUND)
#  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE spdlog::spdlog_header_only)
#ENDIF()

FIND_PACKAGE(Catch2 3 REQUIRED)
IF(Catch2_FOUND)
  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE Catch2::Catch2WithMain)
  #TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE Catch2::Catch2)

  IF(ENABLE_TEST)
    MESSAGE(STATUS "Building tests")
    INCLUDE(CTest)
    INCLUDE(Catch)
    
    catch_discover_tests(${APP_NAME})
  ENDIF()
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

TARGET_COMPILE_DEFINITIONS("${APP_NAME}" PRIVATE ${PROJECT_COMPILE_DEFINITIONS})

TARGET_COMPILE_DEFINITIONS(Processor PRIVATE ${PROJECT_COMPILE_DEFINITIONS})

common_create_install(NAME Processor TARGETS "${APP_NAME}")