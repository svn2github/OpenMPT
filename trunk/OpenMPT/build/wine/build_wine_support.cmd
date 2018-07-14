rem @echo off

set CUR_DIR=%CD%

set BATCH_DIR=%~dp0

set INTDIR=%1

set OUTDIR=%2

cd %BATCH_DIR%
cd ..\..

del /f /q "%OUTDIR%\openmpt-wine-support.zip"
"C:\Program Files\7-Zip\7z.exe" a -tzip -mm=Deflate -mx=9 openmpt-wine-support.zip ^
 LICENSE ^
 include\nlohmann-json\LICENSE.MIT ^
 include\nlohmann-json\include\nlohmann\*.hpp ^
 include\nlohmann-json\include\nlohmann\detail\*.hpp ^
 include\nlohmann-json\include\nlohmann\detail\conversions\*.hpp ^
 include\nlohmann-json\include\nlohmann\detail\input\*.hpp ^
 include\nlohmann-json\include\nlohmann\detail\iterators\*.hpp ^
 include\nlohmann-json\include\nlohmann\detail\output\*.hpp ^
 include\rtkit\rtkit.c ^
 include\rtkit\rtkit.h ^
 common\*.h ^
 common\*.cpp ^
 soundbase\*.h ^
 soundbase\*.cpp ^
 sounddev\*.h ^
 sounddev\*.cpp ^
 misc\*.h ^
 mptrack\wine\*.h ^
 mptrack\wine\*.cpp ^
 mptrack\wine\*.c ^
 build\wine\dialog.sh ^
 build\wine\native_support.mk ^
 build\wine\wine_wrapper.mk ^
 build\wine\wine_wrapper.spec ^
 build\wine\deps\*.txt ^
 || goto error

cd %BATCH_DIR%
move ..\..\openmpt-wine-support.zip "%OUTDIR%\" || goto error

goto noerror

:error
exit 1

:noerror
exit 0
