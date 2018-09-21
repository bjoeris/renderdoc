SET (THIS_PROJECT_NAME gold_reference)
PROJECT(${THIS_PROJECT_NAME})

# Full gold_reference shim
ADD_LIBRARY(${THIS_PROJECT_NAME} SHARED "shim_vulkan.h" "shim_vulkan.cpp" "utils.h" "utils.cpp" "format_conversion.cpp")

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE
                           _UNICODE
                           SAVE_RENDERPASS_BUFFERS)

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

# Screenshot only shim
SET (SCREENSHOT_SHIM_NAME screenshot)
ADD_LIBRARY(${SCREENSHOT_SHIM_NAME} SHARED "shim_vulkan.h" "shim_vulkan.cpp" "utils.h" "utils.cpp" "format_conversion.cpp")

TARGET_COMPILE_DEFINITIONS(${SCREENSHOT_SHIM_NAME} PRIVATE
                           UNICODE
                           _UNICODE)

IF (NOT WIN32)
  SET_TARGET_PROPERTIES(${SCREENSHOT_SHIM_NAME} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden)
ENDIF ()

TARGET_LINK_LIBRARIES(${SCREENSHOT_SHIM_NAME}
                      vulkan
                      helper)

SET_TARGET_PROPERTIES(${SCREENSHOT_SHIM_NAME} PROPERTIES
                      OUTPUT_NAME shim_vulkan
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${SCREENSHOT_SHIM_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${SCREENSHOT_SHIM_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${SCREENSHOT_SHIM_NAME}")
