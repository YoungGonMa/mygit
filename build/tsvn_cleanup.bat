@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

if "%1"=="" goto :usage

set targetDir=%1
echo tsvn-cleanup path: %targetDir%

:main
set tsvn_cmd="C:\Program Files\TortoiseSVN\bin\TortoiseProc.exe"
set opts=/command:cleanup /noui /nodlg /noprogressui /revert /delunversioned /delignored /refreshshell /externals
set target_opt=/path:"%targetDir%"

set cmd=%tsvn_cmd% %opts% %target_opt%
echo %cmd%
%cmd%

:: echo %errorlevel%

goto :end
:usage
echo Usage: tsvn_cleanup.bat target_dir

:end
popd
endlocal
@echo on
