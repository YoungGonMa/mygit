pushd "%~dp0"
call msbuild_cmd.bat "%~dp0..\sln\ecmcmd.vs2017.sln" Release-MT Win32 vc141
popd