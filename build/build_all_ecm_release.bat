@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

:main

call tsvn_cleanup.bat ..

:: Main Release Build
set sln="%~dp0..\sln\DestinyOneRPC.vs2017.sln"
call :build Release-MD Win32 vc141
call :build Release-MD x64 vc141
call :build Release-MT Win32 vc141
call :build Release-MT x64 vc141
:: set sln="%~dp0..\sln\DestinyOneJsonRpc.vs2017.sln"
:: call :build Release-MD Win32 vc141
:: call :build Release-MD x64 vc141
:: call :build Release-MT Win32 vc141
:: call :build Release-MT x64 vc141
set sln="%~dp0..\sln\DestinyOneAgent.vs2017.sln"
call :build Release Win32 vc141
call :build Release x64 vc141
set sln="%~dp0..\sln\DestinyLocalSyncIF.vs2017.sln"
call :build Release-MD Win32 vc141
call :build Release-MD x64 vc141
call :build Release-MT Win32 vc141
call :build Release-MT x64 vc141

:: Main Debug Build
:: set sln="%~dp0..\sln\DestinyOneRPC.vs2017.sln"
:: call :build Debug-MD Win32 vc141
:: call :build Debug-MD x64 vc141
:: call :build Debug-MT Win32 vc141
:: call :build Debug-MT x64 vc141
:: set sln="%~dp0..\sln\DestinyOneJsonRpc.vs2017.sln"
:: call :build Debug-MD Win32 vc141
:: call :build Debug-MD x64 vc141
:: call :build Debug-MT Win32 vc141
:: call :build Debug-MT x64 vc141
:: set sln="%~dp0..\sln\DestinyOneAgent.vs2017.sln"
:: call :build Debug Win32 vc141
:: set sln="%~dp0..\sln\DestinyLocalSyncIF.vs2017.sln"
:: call :build Debug-MD Win32 vc141
:: call :build Debug-MD x64 vc141
:: call :build Debug-MT Win32 vc141
:: call :build Debug-MT x64 vc141

:: DestinySolutionMgr
set sln="%~dp0..\sln\DestinySolutionMgr.vs2017.sln"
call :build Release Win32 vc141
:: call :build Debug Win32 vc141

:: DestinyResDlls
set sln="%~dp0..\sln\DestinyResDlls.vs2017.sln"
call :build Release Win32 vc141
call :build Release x64 vc141
:: call :build Debug Win32 vc141
:: call :build Debug x64 vc141


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
