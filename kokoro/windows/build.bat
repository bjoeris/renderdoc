set "RENDERDOC_ROOT=%KOKORO_ARTIFACTS_DIR%\git\renderdoc"
set "ARTIFACTS_DIR=%KOKORO_ARTIFACTS_DIR%\artifacts"

set /A ERRORS=0

call :BuildRenderdoc Release 1
call :BuildRenderdoc Debug 1

if %ERRORS% gtr 0 EXIT /B 1
exit /B 0

:: Build Renderdoc and preserve the artifacts.
::
:: Args:
::   %1: BUILD_TYPE - Either "Release" or "Debug".
::   %2: CLEAN_BUILD - 0 or 1 indicating whether or not a clean should be
::       performed before building.
:BuildRenderdoc
set "RELEASE_TYPE=%1"
set "CLEAN_BUILD=%2"
set "OUTPUT_DIR=%ARTIFACTS_DIR%\%RELEASE_TYPE%"

call "%RENDERDOC_ROOT%\scripts\build.bat" %RELEASE_TYPE% %CLEAN_BUILD%
mkdir "%OUTPUT_DIR%"
xcopy /s /e "%RENDERDOC_ROOT%\dist\%RELEASE_TYPE%64" "%OUTPUT_DIR%"

if %ERRORLEVEL% gtr 0 set /A ERRORS+=1
EXIT /B 0
