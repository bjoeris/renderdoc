export CC=clang && export CXX=clang++ && rm -rf linux_build && mkdir linux_build && cd linux_build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && ninja && echo "Build complete" 