@echo off

set GOT_REVISION=%1%
set MY_DIR=%CD%



bin\x64\libopenmpt_test.exe || goto error



goto noerror

:error
cd "%MY_DIR%"
exit 1

:noerror
cd "%MY_DIR%"
exit 0
