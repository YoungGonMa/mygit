@echo off

:: Themida를 사용하여 exe,dll 파일을 암호화하기 위한 배치파일 입니다.
:: 임시 작업 파일은 ci/themida 에 생성됩니다.

echo +-----------------+
echo ^| Themida Packing ^|
echo +-----------------+

setlocal enabledelayedexpansion

if "%1" == "" goto usage

:: Themida 설치 경로
set ThemidaPath=C:\Themida

if not exist %ThemidaPath%\Themida.exe (
	echo Themida not installed in this PC, skip.
	goto end
)

set PATH=%PATH%;%ThemidaPath%

:: Themida 설정 파일 경로
::set ThemidaConfigPath=%~dp0
set ThemidaConfigPath=C:\Themida

:: 패킹 파일 소스 경로는 실행 파라미터로, fullPath를 전달받도록 함.

:: 패킹 파일 출력 경로
set OutputPath=%~dp0themida

echo CHECKING VARIABLES...
echo - Current dir = %~dp0
echo - Source file = %1
FOR %%i IN ("%1") DO (
	echo - Source path = %%~di%%~pi%%~ni%%~xi
	set SourceFilePath=%%~di%%~pi%%~ni%%~xi
	echo - Source file = %%~ni%%~xi
	set SourceFileName=%%~ni%%~xi
)
echo - 64bit or not = %2
echo - Output path = %OutputPath%

if not exist %OutputPath% (
	:: 패킹된 파일이 위치할 폴더 생성
	mkdir %OutputPath%
) else (
	:: 이전에 패킹된 파일 삭제
	del /q %OutputPath%\%SourceFileName%
)

echo [Themida] Packing: %SourceFileName%
:: start /w 
themida%2 /protect %ThemidaConfigPath%/themida_config_DestinyECM.tmd /inputfile %SourceFilePath% /outputfile %OutputPath%\%SourceFileName% /shareconsole
echo.
echo ERRORLEVEL = %ERRORLEVEL%

if "%ERRORLEVEL%" == "3" goto ThemidaError3
if "%ERRORLEVEL%" == "2" goto ThemidaError2
if "%ERRORLEVEL%" == "1" goto ThemidaError1
if "%ERRORLEVEL%" == "0" goto ThemidaError0
goto done

:ThemidaError0
echo Application protected successfully.
echo.
echo Replacing original file...
copy /y %OutputPath%\%SourceFileName% %SourceFilePath%
echo ERRORLEVEL = %ERRORLEVEL%
if "%ERRORLEVEL%" == "0" (
	echo - Replace succeeded
	del %OutputPath%\%SourceFileName%
	goto done
) else (
	echo - Failed to replace: ErrorCode = %ERRORLEVEL%
	exit /b -1
)

:ThemidaError1
echo [Themida] ERROR: File already protected!!: %SourceFileName%
goto done

:ThemidaError2
echo [Themida] ERROR: File to protect cannot be opened!!: %SourceFileName%
exit /b -1

:ThemidaError3
:: 참고; Themida 매뉴얼상 에러코드3은 내부에러인데, 실제로 돌려보면 이미 암호화된 파일이 에러코드 3이 나오고 있음.
echo [Themida] ERROR: An internal error occurred while protecting!!: %SourceFileName%
exit /b -1

:done
echo [Themida] Packing complete: %SourceFileName%
goto end

:usage
echo Usage:
echo ------
echo themida_encrypt_file.bat {target_file_path} [64]
echo   ex) themida_encrypt_file.bat C:\bin\DestinyECMAgent.exe
echo       themida_encrypt_file.bat C:\bin\DestinyECMIF64.dll 64
goto end

:end
endlocal
exit /b 0