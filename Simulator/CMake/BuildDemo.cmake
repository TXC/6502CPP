
SET(APP_NAME Demo)

#TARGET_SOURCES(${APP_NAME}
#  PUBLIC  ${PROJECT_SOURCE_DIR}/Simulator.hpp
#)

ADD_EXECUTABLE(${APP_NAME})
TARGET_SOURCES(${APP_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/Demo.cpp
)

INCLUDE(Common)

SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES
  VERSION  "${${PROJECT_NAME}_VERSION}"
  LINKER_LANGUAGE  "${PROJECT_LINKER}"
)

set_compiler_version(
  CXX_STANDARD 20
  OBJCXX 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS ON
  OBJCXX_STANDARD ON
  OBJCXX_STANDARD_REQUIRED ON
)

COMMON_SET_TARGET_PROPERTIES()
TARGET_INCLUDE_DIRECTORIES(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

TARGET_COMPILE_OPTIONS("${APP_NAME}" PRIVATE "-ferror-limit=50")
TARGET_COMPILE_DEFINITIONS("${APP_NAME}" PRIVATE ${PROJECT_COMPILE_DEFINITIONS})

FIND_PACKAGE(Processor REQUIRED)
IF(Processor_FOUND)
  TARGET_LINK_LIBRARIES(${APP_NAME} PUBLIC Processor::Processor)
  TARGET_COMPILE_OPTIONS(Processor PRIVATE "-ferror-limit=50")
  TARGET_COMPILE_DEFINITIONS(Processor PRIVATE ${PROJECT_COMPILE_DEFINITIONS})
ENDIF()

common_create_install(NAME "${APP_NAME}" TARGETS "${APP_NAME}")
