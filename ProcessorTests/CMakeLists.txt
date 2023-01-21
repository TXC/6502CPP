CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
  6502_Processor_Tests
  VERSION 1.0.0
  DESCRIPTION "6502 Processor Tests"
  LANGUAGES CXX
)

SET(APP_NAME tests)

#SET(CMAKE_VERBOSE_MAKEFILE ${CMAKE_VERBOSE_MAKEFILE})
#SET(CMAKE_RULE_MESSAGES ${CMAKE_RULE_MESSAGES})

INCLUDE(Common)

ADD_EXECUTABLE(${APP_NAME})

#COMMON_LANGUAGE_PARAMETERS()
COMMON_SET_PROJECT_FLAGS()

TARGET_SOURCES(tests
  PUBLIC  MainTest.hpp
          ProcessorTests.hpp
          FunctionalProcessorTests.hpp

  PRIVATE MainTest.cpp
          ProcessorTests.cpp
          FunctionalProcessorTests.cpp
)

OPTION(TEST_FUNC "Include Klaus Dormann's Functional Test suite" OFF)
OPTION(TEST_IRQ "Include Klaus Dormann's Interrupt Test suite" OFF)
OPTION(TEST_TIME "Include Ed Spittles's Timing Test suite" OFF)

SET(TestDataDirectory "${CMAKE_CURRENT_SOURCE_DIR}/FunctionalTests/" CACHE PATH "Path to directory where binary test files can be found, must end with slash")

IF(TEST_FUNC)
  ADD_COMPILE_DEFINITIONS(TEST_FUNC=1)
ENDIF(TEST_FUNC)
  
IF(TEST_IRQ)
  ADD_COMPILE_DEFINITIONS(TEST_IRQ=1)
ENDIF(TEST_IRQ)
  
IF(TEST_TIME)
  ADD_COMPILE_DEFINITIONS(TEST_TIME=1)
ENDIF(TEST_TIME)

IF(DEFINED TestDataDirectory AND IS_DIRECTORY ${TestDataDirectory})
  ADD_COMPILE_DEFINITIONS(TESTDIR=${TestDataDirectory})
ELSEIF(DEFINED TestDataDirectory AND NOT IS_DIRECTORY ${TestDataDirectory})
  MESSAGE(SEND_ERROR "[ TESTDIR ] - Path not found - ${TestDataDirectory}")
ENDIF()

FIND_PACKAGE(Processor REQUIRED)
IF(Processor_FOUND)
  TARGET_LINK_LIBRARIES(${APP_NAME} PUBLIC Processor::Processor)
  #TARGET_INCLUDE_DIRECTORIES(${APP_NAME} PUBLIC Processor::Processor)
ENDIF()

#FIND_PACKAGE(fmt REQUIRED CONFIG)
#IF(fmt_FOUND)
#  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE
#    fmt::fmt-header-only
#  )
#ENDIF()
#
#FIND_PACKAGE(spdlog REQUIRED)
#IF(spdlog_FOUND)
#  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE
#    spdlog::spdlog_header_only
#  )
#ENDIF()

FIND_PACKAGE(Catch2 3 REQUIRED)
IF(Catch2_FOUND)
  TARGET_LINK_LIBRARIES(${APP_NAME} PRIVATE
    Catch2::Catch2WithMain
#    Catch2::Catch2
  )

  INCLUDE(CTest)
  INCLUDE(Catch)
  
  catch_discover_tests(${APP_NAME})
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

common_create_install(NAME Processor TARGETS "${APP_NAME}")