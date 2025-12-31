@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

:main
call ..\make_version.h.bat

if not "%1" == "" (
set MSBUILD_DONOT_CLEAN=%1
)

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Main Release Build
set sln="%~dp0..\sln\DestinyLocalSyncIF.vs2017.sln"
:: call :build Release-MD Win32 vc141
:: call :build Release-MD x64 vc141
call :build Release-MT Win32 vc141
call :build Release-MT x64 vc141

set sln="%~dp0..\sln\DestinyResDlls.vs2017.sln"
call :build Release Win32 vc141
call :build Release x64 vc141

set sln="%~dp0..\sln\DestinyOneAgent.vs2017.sln"
call :build Release Win32 vc141

set sln="%~dp0..\sln\DestinyUtilityIF.vs2017.sln"
call :build Release-MT Win32 vc141
call :build Release-MT x64 vc141

set sln="%~dp0..\sln\DestinySUResDlls.vs2017.sln"
call :build Release Win32 vc141
::call :build Release x64 vc141

set sln="%~dp0..\sln\DestinySolutionMgr.vs2017.sln"
call :build Release Win32 vc141
::call :build Release x64 vc141

set sln="%~dp0..\sln\DoMsgMgr.vs2017.sln"
call :build Release Win32 vc141

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Main Debug Build
set sln="%~dp0..\sln\DestinyLocalSyncIF.vs2017.sln"
:: call :build Debug-MD Win32 vc141
:: call :build Debug-MD x64 vc141
call :build Debug-MT Win32 vc141
call :build Debug-MT x64 vc141

set sln="%~dp0..\sln\DestinyResDlls.vs2017.sln"
call :build Debug Win32 vc141
call :build Debug x64 vc141

set sln="%~dp0..\sln\DestinyOneAgent.vs2017.sln"
call :build Debug Win32 vc141

set sln="%~dp0..\sln\DestinyUtilityIF.vs2017.sln"
call :build Debug-MT Win32 vc141
call :build Debug-MT x64 vc141

set sln="%~dp0..\sln\DestinySUResDlls.vs2017.sln"
call :build Debug Win32 vc141
::call :build Debug x64 vc141

set sln="%~dp0..\sln\DestinySolutionMgr.vs2017.sln"
call :build Debug Win32 vc141
::call :build Debug x64 vc141

set sln="%~dp0..\sln\DoMsgMgr.vs2017.sln"
call :build Debug Win32 vc141

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
