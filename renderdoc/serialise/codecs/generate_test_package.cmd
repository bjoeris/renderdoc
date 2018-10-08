@echo off
setlocal enableextensions

rem ESC below has a non-printable character after =...
set ESC=
set GREEN=%ESC%[0;32m
set YELLOW=%ESC%[1;33m
set BOLD=%ESC%[1m
set RESET=%ESC%[0m

if not "%1"=="--help" goto cont
echo.
echo %YELLOW%generate_test_package trace.rdc out_dir [package_name]%RESET%
echo.
echo    %BOLD%Creates Yeti RenderDoc codegen project%RESET%
echo    - %GREEN%trace.rdc%RESET%    - Source RenderDoc trace file to process.
echo    - %GREEN%out_dir%RESET%      - Output directory to write all the package data to. Individual packages go into subsirectories.
echo    - %GREEN%package_name%RESET% - Name of the output package to generate. If omitted, the file name of .RDC file is used.
goto :eof


:cont
if "%1"=="" echo 'package_name' is not specified && goto :eof
if "%2"=="" echo 'out_dir' is not specified && goto :eof
set package_name=%3
if "%3"=="" set package_name=%~n1

set rdc_file=%~2\traces\%package_name%.rdc

set src_dir=%~2\_sources\%package_name%
set yeti_build_dir=%src_dir%\_out\yeti
set linux_build_dir=%src_dir%\_out\linux
set win_build_dir=%src_dir%\_out\win_x64

set package_src_dir=%~2\_package_sources\%package_name%
set package_dir=%~2\packages
set package_tgz=%package_dir%\%package_name%.tgz
set package_tgz_rel_linux=../../packages/%package_name%.tgz

set reference_dir=%~2\reference
set screenshot_dir=%reference_dir%\screenshot
set screenshot_tmp_dir=%screenshot_dir%\%package_name%
set screenshot_file=%package_name%.tgz
set rdoc_auto_capture_dir=%reference_dir%\rdoc_auto_capture

set thumbnails_dir=%~2\thumbnails

echo %GREEN%Copying RDC file from %YELLOW%'%1'%GREEN% to %YELLOW%'%rdc_file%'...%RESET%
mkdir "%~2\traces" 2>nul
copy /y %1 "%rdc_file%" > nul
if %errorlevel% neq 0 exit /b %errorlevel%

rmdir /s /q "%src_dir%" 2>nul
rmdir /s /q "%package_src_dir%" 2>nul
rmdir /s /q "%package_tgz%" 2>nul
mkdir "%package_src_dir%" 2>nul
mkdir "%package_dir%" 2>nul
mkdir "%screenshot_dir%" 2>nul

echo %GREEN%Generating C++ code project in %YELLOW%'%src_dir%'%RESET%
start /w renderdoccmd.exe convert -f "%rdc_file%" -o "%src_dir%.cpp" --shim

echo %GREEN%Extracting reference image to %YELLOW%'%screenshot_dir%\%screenshot_file%'%GREEN%...%RESET%
rmdir /s /q "%screenshot_tmp_dir%" 2>nul
del "%screenshot_dir%\%screenshot_file%" 2>nul
mkdir "%screenshot_tmp_dir%" 2>nul
start /w renderdoccmd.exe thumb "%rdc_file%" -o "%screenshot_tmp_dir%\screenshot_reference.ppm"
if %errorlevel% neq 0 exit /b %errorlevel%
pushd "%screenshot_tmp_dir%" > nul
bash -c "tar -czf ../%screenshot_file% screenshot_reference.ppm" > nul
popd
rmdir /s /q "%screenshot_tmp_dir%" 2>nul
if %errorlevel% neq 0 exit /b %errorlevel%

mkdir "%rdoc_auto_capture_dir%" 2>nul
copy /y "%screenshot_dir%\%screenshot_file%" "%rdoc_auto_capture_dir%" > nul
mkdir "%thumbnails_dir%" 2>nul
start /w renderdoccmd.exe thumb "%rdc_file%" -o "%thumbnails_dir%\%package_name%.png"

echo %GREEN%Building for Yeti...%RESET%
pushd "%src_dir%" > nul
bash -c "source ~/.bashrc && ./build_yeti.sh"
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Building for Windows...%RESET%
pushd "%src_dir%" > nul
call build_vs2015_ninja.bat
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Building for Linux...%RESET%
pushd "%src_dir%" > nul
bash -c "source ~/.bashrc && ./build_xlib.sh"
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Gathering package data...%RESET%
copy /y %yeti_build_dir%\sample_cpp_trace_elf %package_src_dir%\main_yeti > nul
copy /y %yeti_build_dir%\sample_cpp_shim\libshim_vulkan.so %package_src_dir% > nul
copy /y %win_build_dir%\sample_cpp_trace_x64.exe %package_src_dir%\main_win.exe > nul
copy /y %win_build_dir%\sample_cpp_shim\shim_vulkan.dll %package_src_dir% > nul
copy /y %linux_build_dir%\sample_cpp_trace_elf %package_src_dir%\main_linux > nul
mkdir %package_src_dir%\linux
copy /y %linux_build_dir%\sample_cpp_shim\libshim_vulkan.so %package_src_dir%\linux > nul
echo | set /p dummy="LD_LIBRARY_PATH=linux ./main_linux" > %package_src_dir%/run_main_linux.sh
copy /y %src_dir%\sample_cpp_trace\buffer_* %package_src_dir% > nul
copy /y %src_dir%\sample_cpp_trace\pipeline_cache_* %package_src_dir% > nul 2>&1
copy /y %src_dir%\sample_cpp_trace\shader_* %package_src_dir% > nul

echo %GREEN%Building package...%RESET%
pushd "%package_src_dir%" > nul
bash -c "tar -czf %package_tgz_rel_linux% *" > nul
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Package successfully created at %YELLOW%'%package_tgz%'%RESET%
