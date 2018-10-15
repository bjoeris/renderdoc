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
echo %YELLOW%run_trace_package package_name%RESET%
echo.
echo    %BOLD%Downloads a "package_name".rdc file from GCS and runs it on Yeti%RESET%
goto :eof

:cont
if not "%1"=="--shim" goto package
shift
set shim=%1
shift

:package
if "%1"=="" echo 'package_name' is not specified && goto :eof

set package_name=%1
shift

rem if exist "%TEMP%\%package_name%.rdc" goto skip_download
echo %GREEN%Downloading %YELLOW%"%package_name%.rdc"%GREEN% file from GCS...%RESET%
call gsutil cp gs://renderdoc/packages/%package_name%.tgz %TEMP%

:skip_download

echo %GREEN%Setting up gamelet to run the package...%RESET%
yeti ssh put %TEMP%\%package_name%.tgz
yeti ssh shell "tar -xzf /mnt/developer/%package_name%.tgz -C /mnt/developer"

if "%shim%"=="" goto run
echo %GREEN%Downloading and installing %YELLOW%"%shim%"%GREEN% shim...%RESET%
call gsutil cp gs://renderdoc/shims/yeti/%shim%/libshim_vulkan.so %TEMP%
yeti ssh put %TEMP%\libshim_vulkan.so

:run
echo %GREEN%Running package...%RESET%
yeti run --cmd=main_yeti --application=dedd6c2b56af4830a4ee5c53b0598264aps1 %1 %2 %3 %4 %5 %6 %7 %8 %9

