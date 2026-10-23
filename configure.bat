@echo off
setlocal
REM Configure the CMake build (vcpkg restores the dependencies on the first run, so it takes a while).
REM Requires VCPKG_ROOT to point at a vcpkg checkout. Visual Studio and its CMake and Ninja are found by find-msvc.bat,
REM which also enters the shared build environment named by MODS_BUILD_ENV (rule 45).

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

if "%VCPKG_ROOT%"=="" (
	echo VCPKG_ROOT is not set. Point it at your vcpkg checkout and try again.
	exit /b 1
)

cd /d "%~dp0"
git submodule update --init --recursive
cmake --preset x64-RelWithDebInfo
