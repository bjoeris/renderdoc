which cmake && which ninja && export CC=clang && export CXX=clang++ && rm -rf _build_files/linux_build && mkdir -p _build_files/linux_build && cmake -G Ninja --build "" -B_build_files/linux_build -H. -DCMAKE_BUILD_TYPE=Release && ninja -C _build_files/linux_build && echo "Build complete"