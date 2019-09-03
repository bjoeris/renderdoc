@echo off
setlocal enableextensions
set BUILD_TYPE=Release
if NOT "%1" == "" set BUILD_TYPE=%~1
rd /s /q _build_files\win_vs2019x64_ninja 2>nul
mkdir _build_files\win_vs2019x64_ninja
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake.exe -Wno-dev -G Ninja --build "" -H. -B_build_files\win_vs2019x64_ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
ninja -C _build_files\win_vs2019x64_ninja
exit /b %errorlevel%
