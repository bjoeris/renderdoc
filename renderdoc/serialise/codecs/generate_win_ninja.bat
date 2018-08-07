rd /s /q WinNinja
mkdir WinNinja

rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
"C:\Program Files\CMake\bin\cmake.exe" -Wno-dev -G Ninja --build "" -H. -BWinNinja -DCMAKE_BUILD_TYPE=Release

cd WinNinja

ninja

cd ../

pause