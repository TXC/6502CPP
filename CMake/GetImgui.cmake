CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

SET(IMGUI_VERSION 1.89.2)
SET(IMGUI_NAME imgui)
SET(IMGUI_TARGET imgui)

INCLUDE(FetchContent)

FetchContent_Declare(imgui
  #GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
  #GIT_TAG        "v${IMGUI_VERSION}"
  URL            "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip"
  DOWNLOAD_DIR   "${CMAKE_SOURCE_DIR}/imgui"
  SOURCE_DIR     "${CMAKE_SOURCE_DIR}/imgui"
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)  
FetchContent_MakeAvailable(imgui)

CONFIGURE_FILE(
  "${CMAKE_SOURCE_DIR}/CMake/BuildImGui.cmake.in"
  "${CMAKE_SOURCE_DIR}/imgui/CMakeLists.txt"
  @ONLY
)
