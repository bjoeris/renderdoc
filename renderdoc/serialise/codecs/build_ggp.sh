BUILD_TYPE=Release
if [ -n "$1" ]; then
  BUILD_TYPE=$1
fi
which cmake && which ninja && rm -rf _build_files/ggp_build && mkdir -p _build_files/ggp_build && cmake -G Ninja --build "" -B_build_files/ggp_build -H. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_GGP=ON -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake" && ninja -C _build_files/ggp_build && echo "Build complete"
