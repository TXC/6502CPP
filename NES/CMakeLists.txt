CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(
  6502_NES
  VERSION 1.0.0
  DESCRIPTION "Nintento Entertainment System"
  LANGUAGES CXX
)

SET(APP_NAME NES)

OPTION(${APP_NAME}_INSTALL "Generate installation target" ON)

GET_FILENAME_COMPONENT(PARENT_DIR ../ ABSOLUTE)
GET_FILENAME_COMPONENT(PARENT_PARENT_DIR ../../ ABSOLUTE)

INCLUDE(Common)

ADD_LIBRARY(${APP_NAME})
ADD_LIBRARY(${APP_NAME}::${APP_NAME} ALIAS ${APP_NAME})

TARGET_SOURCES(${APP_NAME} PUBLIC
  FILE_SET HEADERS
  FILES 
        APU.hpp
        Audio.hpp
        Cartridge.hpp
        Graphics.hpp
        Mapper_000.hpp
        Mapper_001.hpp
        Mapper_002.hpp
        Mapper_003.hpp
        Mapper_004.hpp
        Mapper_066.hpp
        Mapper.hpp
        NESBus.hpp
        NESResources.hpp
        PPU.hpp
  PRIVATE
        APU.cpp
        Audio.cpp
        Cartridge.cpp
        Graphics.cpp
        Mapper_000.cpp
        Mapper_001.cpp
        Mapper_002.cpp
        Mapper_003.cpp
        Mapper_004.cpp
        Mapper_066.cpp
        Mapper.cpp
        NESBus.cpp
        NESResources.cpp
        PPU.cpp
)

TARGET_COMPILE_FEATURES(${APP_NAME} PUBLIC cxx_std_20)

TARGET_INCLUDE_DIRECTORIES(${APP_NAME} PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<BUILD_INTERFACE:${PARENT_DIR}>"
)

#COMMON_LANGUAGE_PARAMETERS()
COMMON_SET_PROJECT_FLAGS()

set_compiler_version(
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS ON
  TARGET "${APP_NAME}"
)

# BEGIN DEPENDENCIES
FIND_PACKAGE(fmt REQUIRED CONFIG)
IF(fmt_FOUND)
  TARGET_LINK_LIBRARIES("${APP_NAME}" PUBLIC fmt::fmt-header-only)
ENDIF()

FIND_PACKAGE(spdlog REQUIRED)
IF(spdlog_FOUND)
  TARGET_LINK_LIBRARIES("${APP_NAME}" PUBLIC spdlog::spdlog_header_only)
ENDIF()

FIND_PACKAGE(OpenAL REQUIRED)
IF(OpenAL_FOUND)
  IF (APPLE)
    TARGET_LINK_LIBRARIES("${APP_NAME}" PUBLIC "-framework OpenAL")
  ELSE()
    TARGET_LINK_LIBRARIES("${APP_NAME}" PUBLIC OpenAL::OpenAL)
  ENDIF()
ENDIF()

IF(TARGET Processor)
  TARGET_LINK_LIBRARIES(${APP_NAME} PUBLIC Processor)
ELSE()
  FIND_PACKAGE(Processor REQUIRED)
  IF(Processor_FOUND)
    TARGET_LINK_LIBRARIES(${APP_NAME} PUBLIC Processor::Processor)
  ENDIF()
ENDIF()

TARGET_COMPILE_DEFINITIONS(Processor PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
# END DEPENDENCIES

IF(${APP_NAME}_INSTALL)
  COMMON_CREATE_INSTALL(NAME "${APP_NAME}"
    TARGETS "${APP_NAME}"
    INSTALL
  )
ELSE()
  COMMON_CREATE_INSTALL(NAME "${APP_NAME}"
    TARGETS "${APP_NAME}"
  )
ENDIF()
