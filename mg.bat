@echo off
if not defined VCPKG_PATH goto :env_failed

set version=Debug
if /i "%1" == "release" set version=Release

md .\logs

md build\engine\bin\%version%\
cd build\engine
copy ..\xel\.local\bin\*.dll .\bin\%version%\
cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH% -DCMAKE_BUILD_TYPE=%version% -Wno-dev ../../Engine
if "%errorlevel%" NEQ "0" goto :cmake_failed
cmake --build . --config %version%
if "%errorlevel%" NEQ "0" goto :build_failed
ctest.exe --force-new-ctest-process -C %version%
if "%errorlevel%" NEQ "0" goto :test_failed
rd /S/Q .\.local
md .\.local\bin
copy .\bin\%version%\* .\.local\bin\

goto :end

REM Error Cases:
:env_failed
echo environment check failed, make sure VCPKG_PATH points to currect vcpkg path
exit /B

:cmake_failed
echo cmake configuration error !
goto :end

:test_failed
echo Failed to pass test(s) !
goto :end

:build_failed
echo Failed to build target(s) !
goto :end

:end
cd ..\..