rd /s /q Win_VS2017x64
mkdir Win_VS2017x64
rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release
cmake.exe -Wno-dev -G "Visual Studio 15 2017 Win64" --build "" -H. -BWin_VS2017x64
pause