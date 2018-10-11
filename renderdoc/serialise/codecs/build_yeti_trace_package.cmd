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
echo %YELLOW%build_trace_package [--debug^|release] [--run] package_name out_dir%RESET%
echo.
echo    %BOLD%Downloads an "package_name".rdc file from GCS and builds the codegen project for yeti in %RESET%%GREEN%out_dir%RESET%
echo    %GREEN%--debug%RESET% and %GREEN%--release%RESET% specify which flavor to build (%BOLD%release%RESET% is used by default)
echo    If %GREEN%--run%RESET% is specified, also runs it on a currently reserved gamelet
goto :eof

:cont
set config=Release
set run=
set suffix=
if "%1"=="--debug" set config=Debug && set suffix=_DEBUG && shift
if "%1"=="--release" set config=Release && shift
if "%1"=="--run" set run=Y && shift
if "%1"=="" echo 'package_name' is not specified && goto :eof
if "%2"=="" echo 'out_dir' is not specified && goto :eof
set package_name=%1
set src_dir=%~2\_sources\%package_name%
set yeti_build_dir=%src_dir%\_out\yeti
set package_src_dir=%~2\_package_sources\%package_name%
set package_tgz=%~2\%package_name%.tgz
set package_tgz_rel_linux=../../%package_name%.tgz

rem if exist "%TEMP%\%package_name%.rdc" goto skip_download
echo %GREEN%Downloading %package_name%.rdc file from GCS...%RESET%
call gsutil cp gs://renderdoc/traces/%package_name%.rdc %TEMP%

:skip_download
rmdir /s /q "%src_dir%" 2>nul
mkdir "%src_dir%" 2>nul
mkdir "%package_src_dir%" 2>nul

echo %GREEN%Generating C++ code project in %YELLOW%'%src_dir%'%RESET%
start /w renderdoccmd.exe convert -f "%TEMP%\%package_name%.rdc" -o "%src_dir%.cpp" --shim

echo %GREEN%Building for Yeti...%RESET%
pushd "%src_dir%" > nul
bash -c "source ~/.bashrc && ./build_yeti.sh %config%"
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Gathering package data...%RESET%
copy /y %yeti_build_dir%\sample_cpp_trace_elf%suffix% %package_src_dir%\main_yeti > nul
copy /y %yeti_build_dir%\sample_cpp_shim\libshim_vulkan.so %package_src_dir% > nul
copy /y %src_dir%\sample_cpp_trace\buffer_* %package_src_dir% > nul
copy /y %src_dir%\sample_cpp_trace\pipeline_cache_* %package_src_dir% > nul 2>&1
copy /y %src_dir%\sample_cpp_trace\shader_* %package_src_dir% > nul

echo %GREEN%Building package...%RESET%
pushd "%package_src_dir%" > nul
bash -c "tar -czf %package_tgz_rel_linux% *" > nul
popd
if %errorlevel% neq 0 exit /b %errorlevel%

echo %GREEN%Package successfully created at %YELLOW%'%package_tgz%'%RESET%

if %run% == "" goto :eof

echo %GREEN%Setting up gamelet to run the package...%RESET%
yeti ssh put %package_tgz%
yeti ssh shell "tar -xzf /mnt/developer/%package_name%.tgz -C /mnt/developer"

echo %GREEN%Running package...%RESET%
yeti run --cmd=main_yeti --application=dedd6c2b56af4830a4ee5c53b0598264aps1

