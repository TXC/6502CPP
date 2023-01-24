#[===[
.. code-block:: cmake

  set_compiler_version([
    <C|CXX|OBJC|OBJCXX>_STANDARD <value> 
    [<C|CXX|OBJC|OBJCXX>_STANDARD <value>] ...]
    [<C|CXX|OBJC|OBJCXX>_STANDARD_REQUIRED>]
    [<C|CXX|OBJC|OBJCXX>_EXTENSIONS>]
    TARGET [<value> ...]
  )

``C_STANDARD``
* ``90``
* ``99``
* ``11``
* ``17``
* ``23``

``CXX_STANDARD``
* ``98``
* ``11``
* ``14``
* ``17``
* ``20``
* ``23``
* ``26``

``OBJC_STANDARD``
* ``90``
* ``99``
* ``11``

``OBJCXX_STANDARD``
* ``98``
* ``11``
* ``14``
* ``17``
* ``20``
* ``23``
* ``26``

See Also
^^^^^^^^

* :prop_tgt:`LANG_STANDARD`
* :prop_tgt:`LANG_STANDARD_REQUIRED`
* :prop_tgt:`LANG_EXTENSIONS`

]===]
FUNCTION(set_compiler_version)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs TARGET)
  
  IF(CMAKE_C_COMPILER)
    LIST(APPEND oneValueArgs C_STANDARD)
    LIST(APPEND options
      C_STANDARD_REQUIRED
      C_EXTENSIONS
    )
  ENDIF(CMAKE_C_COMPILER)
  
  IF(CMAKE_CXX_COMPILER)
    LIST(APPEND oneValueArgs CXX_STANDARD)
    LIST(APPEND options
      CXX_STANDARD_REQUIRED
      CXX_EXTENSIONS
    )
  ENDIF(CMAKE_CXX_COMPILER)
  
  IF(CMAKE_OBJC_COMPILER)
    LIST(APPEND oneValueArgs OBJC_STANDARD)
    LIST(APPEND options
      OBJC_STANDARD_REQUIRED
      OBJC_EXTENSIONS
    )
  ENDIF(CMAKE_OBJC_COMPILER)
  
  IF(CMAKE_OBJCXX_COMPILER)
    LIST(APPEND oneValueArgs OBJCXX_STANDARD)
    LIST(APPEND options
      OBJCXX_STANDARD_REQUIRED
      OBJCXX_EXTENSIONS
    )
  ENDIF(CMAKE_OBJCXX_COMPILER)

  cmake_parse_arguments("SCV" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # prefer the environment variable OBJC or CC
  FOREACH(var C CXX OBJC OBJCXX)
    IF(CMAKE_${var}_COMPILER AND DEFINED "SCV_${var}_STANDARD")

      MESSAGE(STATUS "<< ${PROJECT_NAME} >> Setting ${var} Standard: ${SCV_${var}_STANDARD}")

      SET(CMAKE_CXX_STANDARD "${SCV_${var}_STANDARD}" PARENT_SCOPE)
      SET(CMAKE_CXX_STANDARD_REQUIRED "${SCV_${var}_STANDARD_REQUIRED}" PARENT_SCOPE)
      SET(CMAKE_CXX_EXTENSIONS "${SCV_${var}_EXTENSIONS}" PARENT_SCOPE)

      SET_PROPERTY(DIRECTORY PROPERTY ${var}_STANDARD )
      SET_PROPERTY(DIRECTORY PROPERTY ${var}_STANDARD_REQUIRED )
      SET_PROPERTY(DIRECTORY PROPERTY ${var}_EXTENSIONS )
      SET_PROPERTY(DIRECTORY APPEND PROPERTY LINKER_LANGUAGE "${var}")
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()


#[=======================================================================[
.. code-block:: cmake

  FUNCTION(common_create_install)(NAME <value>
                                  TARGETS <target>...
                                  [INSTALL])

#]=======================================================================]
FUNCTION(common_create_install)
  set(options INSTALL)
  set(oneValueArgs NAME)
  set(multiValueArgs TARGETS)

  cmake_parse_arguments("CCI" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  IF(NOT DEFINED CCI_NAME)
    MESSAGE(SEND_ERROR "UNABLE TO DETERMINE NAME!")
    MESSAGE(FATAL_ERROR " * NAME: " "${CCI_NAME}")
  ENDIF()

  IF(NOT DEFINED CCI_TARGETS)
    MESSAGE(SEND_ERROR "UNABLE TO DETERMINE TARGETS!")
    MESSAGE(FATAL_ERROR " * TARGETS: " "${CCI_TARGETS}")
  ENDIF()

  SET(TARGET_EXPORT_NAME "${CCI_NAME}Targets")
  IF (INSTALL)
    INCLUDE(GNUInstallDirs)
    INCLUDE(CMakePackageConfigHelpers)
    SET(_CMAKE_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/${CCI_NAME}")

    INSTALL(TARGETS                 "${CCI_TARGETS}"
      EXPORT                        "${TARGET_EXPORT_NAME}"
      LIBRARY DESTINATION           "${CMAKE_INSTALL_LIBDIR}/${CCI_NAME}"
      ARCHIVE DESTINATION           "${CMAKE_INSTALL_LIBDIR}/${CCI_NAME}"
      PRIVATE_HEADER DESTINATION    "${CMAKE_INSTALL_INCLUDEDIR}/${CCI_NAME}"
      PUBLIC_HEADER DESTINATION     "${CMAKE_INSTALL_INCLUDEDIR}/${CCI_NAME}"
      FILE_SET HEADERS DESTINATION  "${CMAKE_INSTALL_INCLUDEDIR}/${CCI_NAME}"
    )
    INSTALL(EXPORT      "${TARGET_EXPORT_NAME}"
          DESTINATION   "${_CMAKE_PATH}"
          NAMESPACE     "${CCI_NAME}::"
          FILE          "${TARGET_EXPORT_NAME}.cmake"
    )
    SET(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

    CONFIGURE_FILE(
      "${CMAKE_SOURCE_DIR}/CMake/Modules.cmake.in"
      "${PROJECT_BINARY_DIR}/${CCI_NAME}Modules.cmake"
      @ONLY
    )

    CONFIGURE_PACKAGE_CONFIG_FILE(
      "${CMAKE_SOURCE_DIR}/CMake/Config.cmake.in"
      "${PROJECT_BINARY_DIR}/${CCI_NAME}Config.cmake"
      INSTALL_DESTINATION "${_CMAKE_PATH}"
    )
    WRITE_BASIC_PACKAGE_VERSION_FILE(
      "${PROJECT_BINARY_DIR}/${CCI_NAME}ConfigVersion.cmake"
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion
      ARCH_INDEPENDENT
    )

    INSTALL(
      FILES "${PROJECT_BINARY_DIR}/${CCI_NAME}Config.cmake"
            "${PROJECT_BINARY_DIR}/${CCI_NAME}ConfigVersion.cmake"
            "${PROJECT_BINARY_DIR}/${CCI_NAME}Modules.cmake"
      DESTINATION "${_CMAKE_PATH}"
    )

    IF(MSVC)
      INSTALL(
        FILES $<TARGET_PDB_FILE:"${CCI_TARGETS}">
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/${CCI_NAME}"
        OPTIONAL
      )
    ENDIF()
  ENDIF()

  EXPORT(TARGETS  ${CCI_TARGETS}
    NAMESPACE     "${CCI_NAME}::"
    FILE          "${PROJECT_BINARY_DIR}/${TARGET_EXPORT_NAME}.cmake"
    EXPORT_LINK_INTERFACE_LIBRARIES
  )
ENDFUNCTION()

MACRO(common_set_project_flags)
  LIST(REMOVE_DUPLICATES PROJECT_COMPILE_FLAGS)
  LIST(REMOVE_DUPLICATES PROJECT_LINK_FLAGS)

  ADD_COMPILE_OPTIONS(PROJECT_COMPILE_FLAGS)
  ADD_LINK_OPTIONS(PROJECT_LINK_FLAGS)
ENDMACRO()

MACRO(common_set_target_properties)
  IF (APPLE)
    SET_TARGET_PROPERTIES(${APP_NAME}
      PROPERTIES
      MACOSX_BUNDLE TRUE
      OUTPUT_NAME "${APP_NAME}"
      MACOSX_BUNDLE_EXECUTABLE_NAME "${APP_NAME}"
      MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
      FOLDER "${APP_NAME}"
      ##RESOURCE icon.icns
      MACOSX_BUNDLE_SHORT_VERSION_STRING ""
      MACOSX_BUNDLE_LONG_VERSION_STRING ""
      ##MACOSX_BUNDLE_ICON_FILE icon.icns
      MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in"
    )
  ELSEIF(WIN32)
    ADD_EXECUTABLE(${APP_NAME})
    SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES
      OUTPUT_NAME "${APP_NAME}"
    )
  ELSEIF(UNIX)
    ADD_EXECUTABLE(${APP_NAME})
    SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES
      OUTPUT_NAME "${APP_NAME}"
    )
  ENDIF()
ENDMACRO()
