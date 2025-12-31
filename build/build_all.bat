@echo off

call build_DestinyOneAgent.bat
:: echo ERRORLEVEL: %ERRORLEVEL%

if ERRORLEVEL 1 (  :: ERRORLEVEL >= 1
    echo ERROR to call build_DestinySolutionMgr.bat
    goto exit
)

call build_DestinySolutionMgr.bat

:exit
