which cmake && which ninja && rm -rf _build_files/yeti_build && mkdir -p _build_files/yeti_build && cmake -G Ninja --build "" -B_build_files/yeti_build -H. -DCMAKE_BUILD_TYPE=Release -DENABLE_YETI=ON -DCMAKE_TOOLCHAIN_FILE="$YETI_SDK_PATH/cmake/yeti.cmake" && ninja -C _build_files/yeti_build && echo "Build complete"