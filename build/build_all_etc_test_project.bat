@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

:main

:: Main Release Build
set sln=..\sln\Test_FileTransferNotiWindow.sln
call :build Debug-MT Win32

goto :end

:build
call msbuild_cmd.bat %sln% %1 %2
if ERRORLEVEL 1 exit 1
exit /b

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
