@echo off
:: setlocal enabledelayedexpansion
setlocal

set thisScriptDir=%~dp0
pushd %thisScriptDir%
set thisScriptFileName=%~nx0

:: for local test
if "%WORKSPACE%" == "" set WORKSPACE=c:\scm-working\_daf

:main
set CLOC="c:\dev-tools\cloc-1.64\cloc-1.64.exe"
set OPTS=--by-file --xml --out="%WORKSPACE%\cloc_result.xml" --exclude-list-file=ci\cloc_exclude-list-file.txt "--force-lang=C/C++ Header,inc"
set TARGET=%WORKSPACE%\daf

cd ..\..
type ci\ci\cloc_exclude-list-file.txt >> ci\cloc_exclude-list-file.txt
%CLOC% %OPTS% %TARGET%

goto :end
:warning
echo %thisScriptFileName% only can run by Jenkins Server

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%
