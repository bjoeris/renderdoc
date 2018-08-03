rd /s /q VS2015_X64
mkdir VS2015_X64

rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release

"C:\Program Files\CMake\bin\cmake.exe" -Wno-dev -G "Visual Studio 14 2015 Win64" --build "" -H. -BVS2015_X64

pause