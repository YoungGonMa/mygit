@echo off
:: setlocal enabledelayedexpansion
setlocal
set thisScriptDir=%~dp0
pushd %thisScriptDir%

if not "%1" == "" (
set MSBUILD_DONOT_CLEAN=%1
)

:main
call msbuild_cmd.bat "%~dp0..\sln\LibDO.vs2017.sln" Release-MD Win32 vc141 %MSBUILD_DONOT_CLEAN%

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
