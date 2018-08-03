export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
rm -rf linux_build
mkdir linux_build
cmake -H. -Blinux_build