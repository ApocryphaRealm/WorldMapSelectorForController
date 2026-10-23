@echo off
setlocal
REM Build the plugin. Runs configure.bat first when the build directory does not exist yet.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

cd /d "%~dp0"
if not exist "build\relwithdebinfo\CMakeCache.txt" call "%~dp0configure.bat"
if errorlevel 1 exit /b 1
cmake --build --preset x64-RelWithDebInfo
