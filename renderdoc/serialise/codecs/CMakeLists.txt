CMAKE_MINIMUM_REQUIRED (VERSION 3.9)

# Disable some of the default cmake build targets, keep debug and release
SET (CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

PROJECT(renderdoc_gen_frame)

SET (WORKING_DIRECTORY_DEBUG      "../../")
SET (WORKING_DIRECTORY_RELEASE    "../../")
SET (MSVS_USERFILE                "${PROJECT_SOURCE_DIR}/Template.user")

SET (PROJECT_PREFIX               "")
SET (MACHINE_POSTFIX              "")
SET (USERFILE_PLATFORM            "")

SET (MACHINE_IS_X64               TRUE)
SET (CMAKE_CXX_STANDARD           11)

IF ("${CMAKE_SIZEOF_VOID_P}"      EQUAL "8")
  SET (MACHINE_IS_X64  TRUE)
ELSEIF ("${CMAKE_SIZEOF_VOID_P}"  EQUAL "4")
  SET (MACHINE_IS_X64 FALSE)
ENDIF ()

IF (WIN32)
  IF (MACHINE_IS_X64)
    SET (MACHINE_POSTFIX            "_x64")
    SET (USERFILE_PLATFORM          "x64")
  ELSE ()
    SET (MACHINE_POSTFIX            "_x86")
    SET (USERFILE_PLATFORM          "Win32")
  ENDIF ()
  IF (MSVC)
    SET (PROJECT_PREFIX               "vs${MSVC_TOOLSET_VERSION}_")
  ENDIF ()
ENDIF ()

GET_FILENAME_COMPONENT(Trace ${CMAKE_CURRENT_SOURCE_DIR} NAME)
STRING(REPLACE " " "_" Trace ${Trace})
PROJECT(${PROJECT_PREFIX}${Trace}${MACHINE_POSTFIX})

OPTION (OPTION_TREAT_WARNINGS_AS_ERRORS
  "Check if you want to treat warnings as errors" FALSE)
OPTION(ENABLE_YETI "Enable Yeti support" OFF)

IF (MSVC)
  IF (OPTION_TREAT_WARNINGS_AS_ERRORS)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  ENDIF (OPTION_TREAT_WARNINGS_AS_ERRORS)
ELSEIF (UNIX)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
ENDIF()

IF (MSVC)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
ENDIF()

SET (EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Build${MACHINE_POSTFIX}")
SET (LIBRARY_OUTPUT_PATH    "${CMAKE_SOURCE_DIR}/Build${MACHINE_POSTFIX}")

SET (DEBUG_POSTFIX "_DEBUG"
     CACHE STRING "Debug Postfitx for lib, samples and tools")
SET (CMAKE_RELEASE_POSTFIX ""
     CACHE STRING "Release Postfitx for lib, samples and tools")
SET (CMAKE_MINSIZEREL_POSTFIX "_MINSIZEREL"
     CACHE STRING "Minimum Size Release Postfitx for lib, samples and tools")
SET (CMAKE_RELWITHDEBINFO_POSTFIX "_RELWITHDEBINFO"
     CACHE STRING "Release With Debug Info Postfitx for lib, samples and tools")

###############################################################################
# Function that changes output names, adding the prefix and postfix
###############################################################################
FUNCTION (SETUP_PROJECT target)
  SET_TARGET_PROPERTIES (${target} PROPERTIES
                         OUTPUT_NAME_DEBUG   ${target}${MACHINE_POSTFIX}${DEBUG_POSTFIX}
                         OUTPUT_NAME_RELEASE ${target}${MACHINE_POSTFIX})
  SET (USER_FILE ${target}.vcxproj.user)
  SET (USERFILE_WORKING_DIRECTORY_DEBUG   ${WORKING_DIRECTORY_DEBUG}${target})
  SET (USERFILE_WORKING_DIRECTORY_RELEASE ${WORKING_DIRECTORY_RELEASE}${target})
  SET (OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${USER_FILE})
  CONFIGURE_FILE (${MSVS_USERFILE} ${OUTPUT_PATH} @ONLY)
ENDFUNCTION ()

# CMAKE DEBUG PRINT OUT
###############################################################################
IF (TRUE)
  MESSAGE (STATUS "CMAKE POINTER SIZE        : ${CMAKE_SIZEOF_VOID_P}")
  MESSAGE (STATUS "CMAKE_COMMAND             : ${CMAKE_COMMAND}")
  MESSAGE (STATUS "CMAKE_BINARY_DIR          : ${CMAKE_BINARY_DIR}")
  MESSAGE (STATUS "CMAKE_CURRENT_BINARY_DIR  : ${CMAKE_CURRENT_BINARY_DIR}")
  MESSAGE (STATUS "CMAKE_CURRENT_LIST_FILE   : ${CMAKE_CURRENT_LIST_FILE}")
  MESSAGE (STATUS "CMAKE_CURRENT_LIST_DIR    : ${CMAKE_CURRENT_LIST_DIR}")
  MESSAGE (STATUS "CMAKE_CURRENT_LIST_LINE   : ${CMAKE_CURRENT_LIST_LINE}")
  MESSAGE (STATUS "CMAKE_CURRENT_SOURCE_DIR  : ${CMAKE_CURRENT_SOURCE_DIR}")
  MESSAGE (STATUS "CMAKE_FILES_DIRECTORY     : ${CMAKE_FILES_DIRECTORY}")
  MESSAGE (STATUS "CMAKE_ROOT                : ${CMAKE_ROOT}")
  MESSAGE (STATUS "CMAKE_SOURCE_DIR          : ${CMAKE_SOURCE_DIR}")
  MESSAGE (STATUS "EXECUTABLE_OUTPUT_PATH    : ${EXECUTABLE_OUTPUT_PATH}")
  MESSAGE (STATUS "LIBRARY_OUTPUT_PATH       : ${LIBRARY_OUTPUT_PATH}")
  MESSAGE (STATUS "PROJECT_NAME              : ${PROJECT_NAME}")
  MESSAGE (STATUS "CMAKE_PROJECT_NAME        : ${CMAKE_PROJECT_NAME}")
  MESSAGE (STATUS "PROJECT_BINARY_DIR        : ${PROJECT_BINARY_DIR}")
  MESSAGE (STATUS "PROJECT_SOURCE_DIR        : ${PROJECT_SOURCE_DIR}")
  MESSAGE (STATUS "CMAKE_CONFIGURATION_TYPES : ${CMAKE_CONFIGURATION_TYPES}")
  MESSAGE (STATUS "DEBUG POSTFIX             : ${DEBUG_POSTFIX}")
  MESSAGE (STATUS "RELEASE POSTFIX           : ${CMAKE_RELEASE_POSTFIX}")
  MESSAGE (STATUS "MSVS_USERFILE             : ${MSVS_USERFILE}")
  MESSAGE (STATUS "CMAKE_CXX_FLAGS           : ${CMAKE_CXX_FLAGS}")
  MESSAGE (STATUS "MSVC VERSION              : ${MSVC_VERSION}")
ENDIF (TRUE)

###############################################################################
# Search for Vulkan
###############################################################################
FIND_PACKAGE (Vulkan)
  MESSAGE (STATUS "Vulkan SDK                : $ENV{VULKAN_SDK}")
IF (Vulkan_FOUND)
  MESSAGE (STATUS "Vulkan Includes           : ${Vulkan_INCLUDE_DIRS}")
  MESSAGE (STATUS "Vulkan Libraries          : ${Vulkan_LIBRARIES}")
ELSE ()
  MESSAGE (FATAL_ERROR "Vulkan not found!")
ENDIF ()

ADD_LIBRARY (vulkan STATIC IMPORTED)

SET_TARGET_PROPERTIES (vulkan PROPERTIES
                       INTERFACE_INCLUDE_DIRECTORIES ${Vulkan_INCLUDE_DIRS}
                       IMPORTED_LOCATION             ${Vulkan_LIBRARIES})

SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sample_cpp_trace)

###############################################################################
# List Cmake projects
###############################################################################
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}")

ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/sample_cpp_trace"
                 "${CMAKE_BINARY_DIR}/sample_cpp_trace")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/helper"
                 "${CMAKE_BINARY_DIR}/helper")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/sample_cpp_shim"
                 "${CMAKE_BINARY_DIR}/sample_cpp_shim")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/gold_reference_shim"
                 "${CMAKE_BINARY_DIR}/gold_reference_shim")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/fix_renderpass"
                 "${CMAKE_BINARY_DIR}/fix_renderpass")
