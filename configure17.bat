@echo off
setlocal
REM Configure the Skyrim 1.7.x build line (CommonLibSSE-NG 7.2.0 from cmake\ports-17, manifest cmake\manifest-17) into
REM build\relwithdebinfo-17. The first run builds CommonLibSSE-NG through vcpkg, so it takes a while.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

if "%VCPKG_ROOT%"=="" (
	echo VCPKG_ROOT is not set. Point it at your vcpkg checkout and try again.
	exit /b 1
)

cd /d "%~dp0"
cmake --preset build-relwithdebinfo-17
