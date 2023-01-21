

SET( _imgui_HEADER_SEARCH_DIRS
  "/usr/include"
  "/usr/local/include"
  "${CMAKE_SOURCE_DIR}/includes"
)
SET( _imgui_LIB_SEARCH_DIRS
  "/usr/lib"
  "/usr/local/lib"
  "${CMAKE_SOURCE_DIR}/lib"
)

# Check environment for root search directory
SET(_imgui_ENV_ROOT $ENV{IMGUI_ROOT})
  IF(NOT IMGUI_ROOT AND _imgui_ENV_ROOT)
  SET(IMGUI_ROOT ${_imgui_ENV_ROOT})
ENDIF()

# Put user specified location at beginning of search
IF( IMGUI_ROOT )
  LIST(INSERT _imgui_HEADER_SEARCH_DIRS 0 "${IMGUI_ROOT}/include")
  LIST(INSERT _imgui_LIB_SEARCH_DIRS 0 "${IMGUI_ROOT}/lib")
ENDIF()

# Search for the header
FIND_PATH(IMGUI_INCLUDE_DIR "imgui/imgui.h"
  PATHS ${_imgui_HEADER_SEARCH_DIRS}
)

# Search for the library
FIND_LIBRARY(IMGUI_LIBRARY imgui
  PATHS ${_imgui_LIB_SEARCH_DIRS}
)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IMGUI DEFAULT_MSG
  IMGUI_LIBRARY IMGUI_INCLUDE_DIR
)
MARK_AS_ADVANCED(IMGUI_INCLUDE_DIR IMGUI_LIB)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IMGUI REQUIRED_VARS IMGUI_INCLUDE_DIR)

if(NOT IMGUI_FOUND)
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
