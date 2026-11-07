@echo off
setlocal
REM Build the Skyrim 1.7.x line. Run configure17.bat first.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

cd /d "%~dp0"
cmake --build --preset relwithdebinfo-17
