@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

:main

call tsvn_cleanup.bat ..

set sln="%~dp0..\sln\All.vs2017.sln"
call :build Debug Win32 vc141
call :build Release Win32 vc141

goto :end

:build
call msbuild_cmd.bat %sln% %1 %2 %3
if ERRORLEVEL 1 exit 1
exit /b

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
