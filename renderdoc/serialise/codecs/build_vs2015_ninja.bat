rd /s /q Win_VS2015x64_Ninja
mkdir Win_VS2015x64_Ninja
rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
cmake.exe -Wno-dev -G Ninja --build "" -H. -BWin_VS2015x64_Ninja -DCMAKE_BUILD_TYPE=Release
cd Win_VS2015x64_Ninja
ninja
cd .. /
pause
