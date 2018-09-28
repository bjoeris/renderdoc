@echo off
setlocal enableextensions
rd /s /q _build_files\win_vs2017x64_ninja 2>nul
mkdir _build_files\win_vs2017x64_ninja
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake.exe -Wno-dev -G Ninja --build "" -H. -B_build_files\win_vs2017x64_ninja -DCMAKE_BUILD_TYPE=Release
ninja -C _build_files\win_vs2017x64_ninja
exit /b %errorlevel%