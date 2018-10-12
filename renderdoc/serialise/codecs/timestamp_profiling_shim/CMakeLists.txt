SET (THIS_PROJECT_NAME timestamp_profiling_shim)

PROJECT(${THIS_PROJECT_NAME})

ADD_LIBRARY(${THIS_PROJECT_NAME} SHARED shim_vulkan.h
                                        shim_vulkan.cpp
                                        shim_vulkan_base.cpp
                                        utils.h
                                        utils.cpp)

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE _UNICODE)
IF (NOT WIN32)
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden)
ENDIF ()

TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                      vulkan
                      helper)

SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                      OUTPUT_NAME shim_vulkan
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}")
