set VCVARSALL_BAT=%VS100COMNTOOLS:Common7\Tools\=VC\vcvarsall.bat%

if #%1# == ##      ( call "%VCVARSALL_BAT%" x86 )
if #%1# == #amd64# ( call "%VCVARSALL_BAT%" amd64 )

