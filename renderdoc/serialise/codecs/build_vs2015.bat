@echo off
setlocal enableextensions
rd /s /q _build_files\win_vs2015x64 2>nul
mkdir _build_files\win_vs2015x64
cmake.exe -Wno-dev -G "Visual Studio 14 2015 Win64" -B_build_files\win_vs2015x64
exit /b %errorlevel%
