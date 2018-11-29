#!/bin/bash

set -ex

# When running inside a Docker sandbox, Bazel records stdout of this script on
# failure but not stderr, which omits error messages and can be confusing. To
# get around this, redirect all of stderr to stdout.
exec 2>&1

readonly BUILD_SOURCE="$1"
readonly BUILD_TARGET="$2"

readonly BUILD_DIR="$(mktemp -d)"
trap "rm -rf '${BUILD_DIR}'" EXIT

cp -r -L "$(pwd)/${BUILD_SOURCE}"/* "${BUILD_DIR}"

pushd "${BUILD_DIR}" &> /dev/null

CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake \
  -DQMAKE_QT5_COMMAND=/ggp/tools/qt/5.11.2/gcc_64/bin/qmake \
  -DRENDERDOC_SWIG_PACKAGE=/ggp/tools/renderdoc/swig-renderdoc-modified-5.zip \
  -DCMAKE_BUILD_TYPE=Release \
  -Bbuild \
  -H.

make -j4 -C build

popd &> /dev/null

readonly TARGET_DIR=$(dirname "${BUILD_TARGET}")
cp ${BUILD_DIR}/build/bin/librenderdoc.so ${TARGET_DIR}
cp ${BUILD_DIR}/build/bin/qrenderdoc ${TARGET_DIR}
cp ${BUILD_DIR}/build/bin/renderdoc_capture.json ${TARGET_DIR}
cp ${BUILD_DIR}/build/bin/renderdoccmd ${TARGET_DIR}
