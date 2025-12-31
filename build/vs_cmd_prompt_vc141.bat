set VCVARSALL_BAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat

if "%1" == "amd64"  ( 
	call "%VCVARSALL_BAT%" x64 8.1
	goto end 
)
if "%1" == "x64"  ( 
	call "%VCVARSALL_BAT%" x64 8.1
	goto end 
)
call "%VCVARSALL_BAT%" x64_x86 8.1

:end

