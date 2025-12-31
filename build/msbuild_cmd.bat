@echo off
:: setlocal enabledelayedexpansion
setlocal
pushd "%~dp0"

if "%1" == "" goto :usage
if "%2" == "" goto :usage

:main
set PLATFORM=%3
set TOOLSET=%4
if "%TOOLSET%" == "" (
    call vs_cmd_prompt.bat
) else (
    if "%TOOLSET%" == "vc141" (
		call %~dp0\vs_cmd_prompt_%TOOLSET%.bat %PLATFORM%
	) else (
		call vs_cmd_prompt_%TOOLSET%.bat
	)
)

set SLN=%1
set CONFIG=%2
set PLATFORM=%3
set TOOLSET=%4
set MSBUILD_TARGET=%5
if "%PLATFORM%" == "" set PLATFORM=Win32
if "%TOOLSET%" == "" set TOOLSET=vc100

echo.
echo.
echo ########################################################################
echo ####
echo #### Starting Visual Studio Build
echo ####   - Solution: %SLN%
echo ####   - Config  : %CONFIG%
echo ####   - Platform: %PLATFORM%
echo ####   - Toolset : %TOOLSET%
echo ####
echo ########################################################################
echo.
echo.

set CFG=/property:Configuration=%CONFIG%
if not "%PLATFORM%" == "" (
    set CFG=%CFG%;Platform=%PLATFORM%
)

set CPU_CNT=/maxcpucount:4
set CMN_OPTS=%SLN% %CFG% %CPU_CNT% /p:PreferredToolArchitecture=x64

if "%MSBUILD_TARGET%" == "clean" @(
MSBuild %CMN_OPTS% /target:Clean
if %ERRORLEVEL% neq 0 goto :end
goto :end
)

if "%MSBUILD_TARGET%" == "build" @(
MSBuild %CMN_OPTS% /target:Build
if %ERRORLEVEL% neq 0 goto :end
goto :end
)

MSBuild %CMN_OPTS% /target:Clean
if %ERRORLEVEL% neq 0 goto :end

MSBuild %CMN_OPTS% /target:Build
if %ERRORLEVEL% neq 0 goto :end

goto :end

:usage
echo Usage:
echo ------
echo msbuild_cmd.bat {solution_file_path} {config_name} {platform_name} [vc100|vc140]
echo   ex) msbuild_cmd.bat my_project.sln Release-MD Win32
echo       msbuild_cmd.bat my_project.sln Release-MD x64 vc140

:end
popd
endlocal
@echo on
@exit /b %ERRORLEVEL%


