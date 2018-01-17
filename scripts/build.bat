@echo on

:: This scripts requires the following parameters:
::    %1: Represents the release type. It has two following
::        values: Release, Debug
::    %2: A value of 0 or 1. 1 means full clean. 0 means use existing
::        build artifacts such as obj files.
:: The following parameter is optional:
::    %3: The GCS_URL_BASE link to upload the renderdoc build artifacts.
::
:: The following tools are required in order to build the renderdoc:
::    1) Visual Studio 2015
::    2) gsutil application to upload the build artifacts.

set RELEASE_TYPE=%1
if "%RELEASE_TYPE%" == "" (
   echo ERROR: Release type not defined as a first parameter of this script.
   exit /b 1
)

:: In the renderdoc visual studio project, the author defined the debug build
:: as Development.
if "%RELEASE_TYPE%" == "Release" (
   set CONFIG=Release
) else (
   set CONFIG=Development
)

set CLEAN_BUILD=%2
if "%CLEAN_BUILD%" == "" (
    echo "ERROR: Clean build was not specified in the second script parameter."
    exit /b 1
)

:: Optional: Upload the artifacts to renderdoc/client under the GCS_URL_BASE
:: path.
set GCS_URL_BASE=%3

set RENDERDOC_ROOT_DIR=%~dp0\..

:: The Visual studio project to build renderdoc
set RENDER_DOC_VS_PROJECT=%RENDERDOC_ROOT_DIR%\renderdoc.sln

set RENDER_DOC_BUILD_OPTIONS=/m ^
    /p:Configuration=%CONFIG% ^
    /p:Platform=x64 ^
    /p:BuildInParallel=true

:: Load the required environment variables to locate the required build tools
call "%PROGRAMFILES(X86)\Microsoft Visual Studio 14.0\Common7\Tools\VsMSBuildCmd.bat" || exit /b 1

:: Remove any artifacts left from the previous build
:if "%CLEAN_BUILD%" == "1" (
   msbuild.exe %RENDER_DOC_VS_PROJECT% %RENDER_DOC_BUILD_OPTIONS% /t:clean || exit /b 1
)

:: Build the render doc
msbuild.exe %RENDER_DOC_VS_PROJECT% %RENDER_DOC_BUILD_OPTIONS% || exit /b 1

:: Create the distribution folder to place the artifacts
set DESTINATION_DIR=%RENDERDOC_ROOT_DIR%\dist\%RELEASE_TYPE%64

rmdir /s /q "%DESTINATION_DIR%"
mkdir "%DESTINATION_DIR%"

xcopy /S /F /Y ^
   "%RENDERDOC_ROOT_DIR%\x64\%CONFIG%\*" ^
   "%DESTINATION_DIR%\" ^
   || exit /b 1

rmdir /S /Q "%DESTINATION_DIR%\obj"

copy ^
   "%PROGRAMFILES(X86)%\Windows Kits\8.1\Redist\D3D\x64\d3dcompiler_47.dll" ^
   "%DESTINATION_DIR%\" ^
   || exit /b 1

copy ^
   "%RENDERDOC_ROOT_DIR%\LICENSE.md" ^
   "%DESTINATION_DIR%\" ^
   || exit /b 1

pushd %DESTINATION_DIR%
for %%f in (*.ipdb *.iobj *.exp *.lib *.metagen *.vshost.*) do (
   del /F %%f
)
popd

if not "%GCS_URL_BASE%" == "" (
   gsutil -m -q cp -r %DESTINATION_DIR% "%GCS_URL_BASE%/renderdoc/client"
)

