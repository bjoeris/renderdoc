which cmake && which ninja && rm -rf yeti_build && mkdir yeti_build && cd yeti_build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_YETI=ON -DCMAKE_TOOLCHAIN_FILE="$YETI_SDK_PATH/cmake/yeti.cmake" .. && ninja && echo "Build complete"