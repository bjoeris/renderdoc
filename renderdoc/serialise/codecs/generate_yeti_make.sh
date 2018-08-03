rm -rf yeti_build
mkdir yeti_build
cmake -H. -Byeti_build -DCMAKE_TOOLCHAIN_FILE="$YETI_SDK_PATH/cmake/yeti.cmake" -DENABLE_YETI=ON
