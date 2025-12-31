@echo off
:: setlocal enabledelayedexpansion
setlocal
set thisScriptDir=%~dp0
pushd %thisScriptDir%

:main
call msbuild_cmd.bat "%~dp0..\sln\ECMIF_unit_test.vs2017.sln" Release Win32 vc141

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
