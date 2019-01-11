#!/bin/bash
#
# Build RenderDoc for Linux.
set -ex

readonly ARTIFACTS_DIR="${KOKORO_ARTIFACTS_DIR}/artifacts"
readonly BAZEL="/home/kbuilder/bin/bazel"
readonly BAZEL_BUILD_OPTS=(
  --auth_credentials="${KOKORO_KEYSTORE_DIR}/71274_kokoro_service_key_json"
  --linkopt="-Wl,--build-id=md5"
  --strip="never"
)
readonly BUILD_TARGETS=(
  "//cloud/libs:libyeti.so"
  "//cloud/yeti:yeti_internal_sdk_headers"
  "//cloud/yeti_c:yeti_c_internal_sdk_headers"
  "//common/libs/version:version_internal_sdk_headers"
  "//graphics/packaging:sysroot_pkg"
)
readonly RENDERDOC_ARTIFACTS=(
  "bin/renderdoccmd"
  "lib/librenderdoc.so"
  "lib/renderdoc_capture.json"
)
readonly RENDERDOC_ROOT="${KOKORO_ARTIFACTS_DIR}/git/renderdoc"
readonly YETI_ROOT="${KOKORO_ARTIFACTS_DIR}/git/yeti"


# Untar the BaseSDK into the renderdoc repository.
#
# Globals:
#   RENDERDOC_ROOT: Path to the directory containing the renderdoc repository.
#   KOKORO_GFILE_DIR: Path to outside resources imported to the Kokoro build.
#                     This is passed in by the Kokoro build system.
function setup_base_sdk() {
  local -r sdk="${RENDERDOC_ROOT}/tools/YetiSDK-latest"
  mkdir -p "${sdk}"
  tar -xzf \
    "${KOKORO_GFILE_DIR}/YetiSDKBaseInternal-linux.tar.gz" --strip=1 -C ${sdk}

  export BASE_SDK_HEADERS="${RENDERDOC_ROOT}/tools/YetiSDK-latest/sysroot/usr/include"
  export LD_LIBRARY_PATH="${RENDERDOC_ROOT}/tools/YetiSDK-latest/sysroot/lib"
  export PATH="${RENDERDOC_ROOT}/tools/YetiSDK-latest/toolchain/bin:${PATH}"
}


# Build the yeti/yeti components that are required for the RenderDoc build.
#
# Globals:
#   BASE_SDK_HEADERS: Path to sysroot include directory.  Set by
#                     setup_base_sdk.
#   BAZEL: Path to bazel executable.
#   BAZEL_BUILD_OPTS: Bazel arguments.
#   BAZEL_FLAGS: Bazel startup options. This is set by the bazel installer.
#   LD_LIBRARY_PATH: Path to the sysroot lib directory.  Set by setup_base_sdk.
#   YETI_ROOT: Path to the top level of the local copy of the yeti/yeti
#              repository.
function build_yeti_components() {
  pushd "${YETI_ROOT}" &> /dev/null

  ${BAZEL} "${BAZEL_FLAGS[@]}" build "${BAZEL_BUILD_OPTS[@]}" \
    "${BUILD_TARGETS[@]}"

  bazel_bin=$(
    ${BAZEL} "${BAZEL_FLAGS[@]}" info "${BAZEL_BUILD_OPTS[@]}" bazel-bin)
  bazel_genfiles=$(
    ${BAZEL} "${BAZEL_FLAGS[@]}" info "${BAZEL_BUILD_OPTS[@]}" bazel-genfiles)

  cp -a "${bazel_bin}/cloud/libs/libyeti.so" "${LD_LIBRARY_PATH}/"
  cp -a "${bazel_genfiles}/cloud/yeti_c/internal_sdk/yeti_c" \
    "${BASE_SDK_HEADERS}/"
  cp -a "${bazel_genfiles}/common/libs/version/internal_sdk/yeti_c/version.h" \
    "${BASE_SDK_HEADERS}/yeti_c/"
  tar -C "${BASE_SDK_HEADERS}/" -zxvf \
    "${bazel_bin}/graphics/packaging/sysroot_pkg.tar.gz" \
    ./usr/include/vulkan --strip-components=3

  popd &> /dev/null
}


# Build RenderDoc for the specified release type.
#
# Globals:
#   ARTIFACTS_DIR: Path to output RenderDoc artifacts.
#   RENDERDOC_ARTIFACTS: Artifacts that will be retained.
#   RENDERDOC_ROOT: Path to the directory containing the RenderDoc repository.
# Args:
#   RELEASE_TYPE: Release or Debug
function build_renderdoc() {
  RELEASE_TYPE="$1"

  local RENDERDOC_BUILD_DIR="${RENDERDOC_ROOT}/${RELEASE_TYPE}"
  rm -rf "${RENDERDOC_BUILD_DIR}"
  mkdir -p "${RENDERDOC_BUILD_DIR}"

  pushd "${RENDERDOC_BUILD_DIR}" &> /dev/null

  # Generate the build files and then build RenderDoc.
  cmake \
    -DCMAKE_TOOLCHAIN_FILE="${RENDERDOC_ROOT}/tools/YetiSDK-latest/cmake/yeti.cmake" \
    -DCMAKE_BUILD_TYPE="${RELEASE_TYPE}" \
    -DCMAKE_INSTALL_PREFIX=/usr/local/cloudcast \
    -DCMAKE_INSTALL_RPATH=/usr/local/cloudcast/lib \
    -DENABLE_YETI=ON \
    -DENABLE_GL=OFF \
    -DENABLE_QRENDERDOC=OFF \
    -DENABLE_XCB=OFF \
    -DENABLE_XLIB=OFF \
    "${RENDERDOC_ROOT}"

  cmake --build . -- -j

  # Save the RenderDoc artifacts.
  mkdir -p "${ARTIFACTS_DIR}/renderdoc/${RELEASE_TYPE}"
  for renderdoc_artifact in "${RENDERDOC_ARTIFACTS[@]}"
  do
    cp "${RENDERDOC_BUILD_DIR}/${renderdoc_artifact}" \
      "${ARTIFACTS_DIR}/renderdoc/${RELEASE_TYPE}/"
  done

  popd &> /dev/null
}


function main() {
  # Workaround for b/111569209.
  # If this is running in UBUNTU, ensure that bazel uses IPv6.
  if [[ "${KOKORO_JOB_CLUSTER}" == "UBUNTU" ]]; then
    export JAVA_TOOL_OPTIONS="-Djava.net.preferIPv6Addresses=true"
  else
    # On GCP_UBUNTU, we need a token in order for some later bazel steps to fetch
    # remote dependencies from git-on-borg.
    git clone https://gerrit.googlesource.com/gcompute-tools
    ./gcompute-tools/git-cookie-authdaemon
  fi

  # Install the current version of bazel that is being used to build yeti/yeti.
  readonly BAZEL_VERSION='0.21.0'
  source "${YETI_ROOT}/kokoro/ubuntu/install_bazel.sh"

  gcloud auth activate-service-account \
    --key-file "${KOKORO_KEYSTORE_DIR}/71274_kokoro_service_key_json"
  setup_base_sdk
  build_yeti_components
  build_renderdoc Release
  build_renderdoc Debug
}


main
