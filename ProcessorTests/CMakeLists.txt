CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
  6502_Processor_Tests
  VERSION 1.0.0
  DESCRIPTION "6502 Processor Tests"
  LANGUAGES CXX
)

SET(LINKER_LANGUAGE CXX)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS ON)
SET(THREADS_PREFER_PTHREAD_FLAD ON)

SET(CMAKE_VERBOSE_MAKEFILE ${CMAKE_VERBOSE_MAKEFILE})
SET(CMAKE_RULE_MESSAGES ${CMAKE_RULE_MESSAGES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")

ADD_EXECUTABLE(tests
  MainTest.cpp
  ProcessorTests.cpp
  ProcessorTests.hpp
  FunctionalProcessorTests.cpp
)

SET_TARGET_PROPERTIES(tests PROPERTIES VERSION  "${${PROJECT_NAME}_VERSION}"
                               LINKER_LANGUAGE  "${LINKER_LANGUAGE}"
                            CMAKE_CXX_STANDARD  "${CMAKE_CXX_STANDARD}"
                   CMAKE_CXX_STANDARD_REQUIRED  "${CMAKE_CXX_STANDARD_REQUIRED}"
                          CMAKE_CXX_EXTENSIONS  "${CMAKE_CXX_EXTENSIONS}"
                   THREADS_PREFER_PTHREAD_FLAD  "${THREADS_PREFER_PTHREAD_FLAD}"
)

TARGET_LINK_LIBRARIES(tests PUBLIC Processor)
TARGET_INCLUDE_DIRECTORIES(tests PUBLIC Processor)

FIND_PACKAGE(Catch2 3 REQUIRED)
TARGET_LINK_LIBRARIES(tests PRIVATE Catch2::Catch2WithMain)

INCLUDE(CTest)
INCLUDE(Catch)

catch_discover_tests(tests)