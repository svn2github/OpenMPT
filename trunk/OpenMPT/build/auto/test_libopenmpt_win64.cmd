@echo off

set BATCH_DIR=%~dp0
cd %BATCH_DIR%
cd ..\..

set GOT_REVISION=%1%
set MY_DIR=%CD%



bin\x64\libopenmpt_test.exe || goto error



goto noerror

:error
cd "%MY_DIR%"
exit /B 1

:noerror
cd "%MY_DIR%"
exit /B 0
