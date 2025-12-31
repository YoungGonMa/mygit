cd %WORKSPACE%\_projects
set MY_SVN_REVISION=%SVN_REVISION%

%SYSTEMDRIVE%
cd %WINDIR%
set WINDBG_PATH=C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64\
set PRODUCT_SYMBOL_PATH=D:\Symbols\ProductSymbols
set PATH=C:\Perl64\bin;%PATH%

call "%WINDBG_PATH%srcsrv\svnindex.cmd" /debug /source="%WORKSPACE%\_projects" /symbols="%WORKSPACE%\_projects\bin"
call "%WINDBG_PATH%srcsrv\svnindex.cmd" /debug /source="%WORKSPACE%\_projects" /symbols="%WORKSPACE%\_projects\bin\cloudium"
"%WINDBG_PATH%srcsrv\srctool.exe" "%WORKSPACE%\_projects\bin\*.pdb"
"%WINDBG_PATH%srcsrv\srctool.exe" "%WORKSPACE%\_projects\bin\cloudium\*.pdb"
"%WINDBG_PATH%symstore.exe" add /f "%WORKSPACE%\_projects\bin\*.*" /s "%PRODUCT_SYMBOL_PATH%" /t "%JOB_NAME%" /v "%BUILD_DISPLAY_NAME%" /c "rev.%MY_SVN_REVISION%_%BUILD_TAG%"
"%WINDBG_PATH%symstore.exe" add /f "%WORKSPACE%\_projects\bin\cloudium\*.*" /s "%PRODUCT_SYMBOL_PATH%" /t "%JOB_NAME%_Cloudium" /v "%BUILD_DISPLAY_NAME%" /c "rev.%MY_SVN_REVISION%_%BUILD_TAG%"