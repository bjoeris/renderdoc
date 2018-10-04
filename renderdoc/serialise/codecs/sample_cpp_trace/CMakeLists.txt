IF(WIN32)
SET (THIS_PROJECT_NAME sample_cpp_trace)
ELSE ()
SET (THIS_PROJECT_NAME sample_cpp_trace_elf)
ENDIF ()

PROJECT(${THIS_PROJECT_NAME})

FILE(GLOB HEADERS "*.h")
FILE(GLOB SOURCES "*.cpp")
SET(SHADERS )
SET(TEMPLATES main_win.cpp main_yeti.cpp main_xlib.cpp)

SOURCE_GROUP ( "Source Files"   FILES ${SOURCES} )
SOURCE_GROUP ( "Template Files" FILES ${TEMPLATES} )
SOURCE_GROUP ( "Header Files"   FILES ${HEADERS} )
SOURCE_GROUP ( "Shader Files"   FILES ${SHADERS} )

ADD_EXECUTABLE(${THIS_PROJECT_NAME} ${TEMPLATES} ${SOURCES}
                                    ${HEADERS} ${SHADERS})

SETUP_PROJECT(${THIS_PROJECT_NAME})

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE _UNICODE)

SET(SHIM_DEPS )
IF (ENABLE_SHIM)
  SET(SHIM_DEPS sample_cpp_shim)
ENDIF()

IF (WIN32)
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_WIN32_KHR
                             _CRT_SECURE_NO_DEPRECATE)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        vulkan
                        helper
                        comctl32
                        rpcrt4
                        winmm
                        advapi32
                        wsock32
                        Dbghelp
                        ${SHIM_DEPS})
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /STACK:67108864"
                        LINK_FLAGS_DEBUG   "/SUBSYSTEM:WINDOWS /STACK:67108864")
ELSEIF (ENABLE_YETI)
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_YETI_GOOGLE
                             __yeti__)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        libyeti.so
                        libdl.so
                        vulkan
                        helper
                        ${SHIM_DEPS})
ELSE ()
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_XLIB_KHR
                             __linux__)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        libX11.so
                        libdl.so
                        helper
                        vulkan
                        ${SHIM_DEPS})
ENDIF ()