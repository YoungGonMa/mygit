@echo off
:: setlocal enabledelayedexpansion
setlocal
set thisScriptDir=%~dp0
pushd %thisScriptDir%

:main
call ..\make_version.h.bat
set sln="%~dp0..\sln\DestinyOneRPC.vs2017.sln"

if not "%1" == "" (
set MSBUILD_DONOT_CLEAN=%1
)

call :build Debug-MD Win32 vc141
call :build Debug-MT Win32 vc141
call :build Debug-MD x64 vc141
call :build Debug-MT x64 vc141
call :build Release-MD Win32 vc141
call :build Release-MT Win32 vc141
call :build Release-MD x64 vc141
call :build Release-MT x64 vc141

goto :end

:build
call msbuild_cmd.bat %sln% %1 %2 %3 %MSBUILD_DONOT_CLEAN%
if ERRORLEVEL 1 exit 1
exit /b

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
