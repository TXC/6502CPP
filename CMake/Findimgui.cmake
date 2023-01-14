SET(_IMGUI_SEARCH_NAMES
  "imgui"
  "../imgui"
  "../../imgui"
  "../../../imgui"
  "../../../../imgui"
  "../../../../../imgui"
  "../../../../../../imgui"
  "../../../../../../../imgui"
)
SET(IMGUI_LIB)   # Link

find_path(IMGUI_INCLUDE_DIR olcPixelGameEngine.hpp
  PATHS ${_PGE_SEARCH_NAMES}
  PATH_SUFFIXES "lib" "build" "include"
)
message(STATUS "ImGui header found at: ${IMGUI_INCLUDE_DIR}")

mark_as_advanced(
  IMGUI_INCLUDE_DIR
  IMGUI_LIB
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IMGUI REQUIRED_VARS IMGUI_INCLUDE_DIR)

if(IMGUI_FOUND)
  # OpenGL
  FIND_PACKAGE(OpenGL REQUIRED)
  if (OPENGL_FOUND)
    if (LINUX OR WIN32)
      LIST(APPEND PixelGameEngine_LIB
        OpenGL::OpenGL
        OpenGL::GL
      #  OpenGL::GLU
        OpenGL::GLX
      #  OpenGL::EGL
      )
    endif(LINUX OR WIN32)
    if (APPLE)
      LIST(APPEND PixelGameEngine_LIB ${OPENGL_LIBRARIES})
    endif(APPLE)
  endif()
  
  if (LINUX OR APPLE)
    # X11
    FIND_PACKAGE(X11 REQUIRED)
    if(X11_FOUND)
      LIST(APPEND PixelGameEngine_LIB ${X11_LIBRARIES})
    endif(X11_FOUND)
  
    # PNG
    FIND_PACKAGE(PNG REQUIRED)
    if(PNG_FOUND)
      LIST(APPEND PixelGameEngine_INCLUDE_DIR ${PNG_INCLUDE_DIRS})
      LIST(APPEND PixelGameEngine_LIB PNG::PNG)
    endif()
  endif()

  if (APPLE)
    # Carbon
    FIND_LIBRARY(CARBON_LIBRARY Carbon REQUIRED)
    if (GLUT_FOUND)
      LIST(APPEND PixelGameEngine_LIB ${CARBON_LIBRARY})
    endif()

    # GLUT (OpenGL Utilities)
    FIND_PACKAGE(GLUT REQUIRED)
    if (GLUT_FOUND)
      LIST(APPEND PixelGameEngine_LIB GLUT::GLUT)
    endif()
  endif(APPLE)

  if(WIN32)
    LIST(APPEND PixelGameEngine_INCLUDE_DIR ${WinSDK})
  endif(WIN32)

  if(NOT TARGET PixelGameEngine::PixelGameEngine)
    ADD_LIBRARY(PixelGameEngine::PixelGameEngine INTERFACE IMPORTED)
    LIST(APPEND PixelGameEngine_LIB PixelGameEngine::PixelGameEngine)
  endif()

  if(NOT TARGET PixelGameEngine::Extensions AND TARGET PixelGameEngine::PixelGameEngine)
    ADD_LIBRARY(PixelGameEngine::Extensions INTERFACE IMPORTED)
    LIST(APPEND PixelGameEngine_LIB PixelGameEngine::Extensions)
  endif()


  if(NOT TARGET PixelGameEngine::Utilities AND TARGET PixelGameEngine::PixelGameEngine)
    ADD_LIBRARY(PixelGameEngine::Utilities INTERFACE IMPORTED)
    LIST(APPEND PixelGameEngine_LIB PixelGameEngine::Utilities)
  endif()

  if(TARGET PixelGameEngine::PixelGameEngine)
    SET_TARGET_PROPERTIES(PixelGameEngine::PixelGameEngine
      PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 INTERFACE_INCLUDE_DIRECTORIES "${PixelGameEngine_INCLUDE_DIR}"
                 INTERFACE_LINK_LIBRARIES "${PixelGameEngine_LIB}"
    )
  endif()

  if(TARGET PixelGameEngine::Extensions)
    SET_TARGET_PROPERTIES(PixelGameEngine::Extensions
      PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
              INTERFACE_LINK_LIBRARIES "${PixelGameEngine_LIB}"
              INTERFACE_INCLUDE_DIRECTORIES "${PixelGameEngine_INCLUDE_DIR}"
    )
  endif()

  if(TARGET PixelGameEngine::Utilities)
    SET_TARGET_PROPERTIES(PixelGameEngine::Utilities
      PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 INTERFACE_LINK_LIBRARIES "${PixelGameEngine_LIB}"
                 INTERFACE_INCLUDE_DIRECTORIES "${PixelGameEngine_INCLUDE_DIR}"
    )
  endif()
endif()
