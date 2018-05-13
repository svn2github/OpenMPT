@echo off

if not "x%1" == "xauto" (
	echo "WARNING: This script will unconditionally remove all files from the destination directories."
	echo "This script requires Windows 7 or later (because of PowerShell)."
	echo "This script requires 7-zip in 'C:\Program Files\7-Zip\' (the default path for a native install)."
	echo "When running from a Subversion working copy, this script requires at least Subversion 1.7 (because it removes subdirectories which should not contain .svn metadata)."
	pause
)

if "x%2" == "xnodownload" (
 set MPT_DOWNLOAD=no
)
if not "x%2" == "xnodownload" (
 set MPT_DOWNLOAD=yes
)

set MY_DIR=%CD%
set BATCH_DIR=%~dp0
cd %BATCH_DIR% || goto error
cd .. || goto error
goto main

:download_and_unpack
 set MPT_GET_DESTDIR="%~1"
 set MPT_GET_URL="%~2"
 set MPT_GET_FILE="%~3"
 set MPT_GET_SUBDIR="%~4"
 set MPT_GET_UNPACK_INTERMEDIATE="%~5"
 if "%MPT_DOWNLOAD%" == "yes" (
  if not exist "build\externals\%~3" (
   powershell -Command "(New-Object Net.WebClient).DownloadFile('%MPT_GET_URL%', 'build/externals/%~3')" || exit /B 1
  )
 )
 cd build\externals || exit /B 1
 if not "%~5" == "-" (
  "C:\Program Files\7-Zip\7z.exe" x -y "%~3" || exit /B 1
 )
 cd ..\.. || exit /B 1
 cd include || exit /B 1
 if exist %MPT_GET_DESTDIR% rmdir /S /Q %MPT_GET_DESTDIR%
 if "%~4" == "." (
  mkdir %MPT_GET_DESTDIR%
  cd %MPT_GET_DESTDIR% || exit /B 1
  if "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\..\build\externals\%~3" || exit /B 1
  )
  if not "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\..\build\externals\%~5" || exit /B 1
  )
  cd .. || exit /B 1
 )
 if not "%~4" == "." (
  if "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\build\externals\%~3" || exit /B 1
  )
  if not "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\build\externals\%~5" || exit /B 1
  )
  choice /C y /N /T 2 /D y
  if not "%~4" == "%~1" (
   move /Y "%~4" %MPT_GET_DESTDIR% || exit /B 1
  )
 )
 cd .. || exit /B 1
exit /B 0
goto error

:unpack
 set MPT_GET_DESTDIR="%~1"
 set MPT_GET_URL="%~2"
 set MPT_GET_FILE="%~3"
 set MPT_GET_SUBDIR="%~4"
 set MPT_GET_UNPACK_INTERMEDIATE="%~5"
 copy /Y build\externals-mirror\%MPT_GET_URL% build\externals\%~3 || exit /B 1
 cd build\externals || exit /B 1
 if not "%~5" == "-" (
  "C:\Program Files\7-Zip\7z.exe" x -y "%~3" || exit /B 1
 )
 cd ..\.. || exit /B 1
 cd include || exit /B 1
 if exist %MPT_GET_DESTDIR% rmdir /S /Q %MPT_GET_DESTDIR%
 if "%~4" == "." (
  mkdir %MPT_GET_DESTDIR%
  cd %MPT_GET_DESTDIR% || exit /B 1
  if "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\..\build\externals\%~3" || exit /B 1
  )
  if not "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\..\build\externals\%~5" || exit /B 1
  )
  cd .. || exit /B 1
 )
 if not "%~4" == "." (
  if "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\build\externals\%~3" || exit /B 1
  )
  if not "%~5" == "-" (
   "C:\Program Files\7-Zip\7z.exe" x -y "..\build\externals\%~5" || exit /B 1
  )
  choice /C y /N /T 2 /D y
  if not "%~4" == "%~1" (
   move /Y "%~4" %MPT_GET_DESTDIR% || exit /B 1
  )
 )
 cd .. || exit /B 1
exit /B 0
goto error

:main
if not exist "build\externals" mkdir "build\externals"



rem call :download_and_unpack "genie" "https://github.com/bkaradzic/GENie/archive/78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377" "-" || goto error
call :unpack "genie" "genie\GENie-78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377.zip" "GENie-78817a9707c1a02e845fb38b3adcc5353b02d377" "-" || goto error

xcopy /E /I /Y build\genie\genie\build\vs2015 include\genie\build\vs2015 || goto error
xcopy /E /I /Y build\genie\genie\build\vs2017 include\genie\build\vs2017 || goto error

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
 call build\auto\setup_vs2017.cmd || goto error
 cd include\genie\build\vs2017 || goto error
 devenv genie.sln /Upgrade || goto error
 devenv genie.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto geniedone
)
if exist "C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
 call build\auto\setup_vs2017.cmd || goto error
 cd include\genie\build\vs2017 || goto error
 devenv genie.sln /Upgrade || goto error
 devenv genie.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto geniedone
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
 call build\auto\setup_vs2015.cmd || goto error
 cd include\genie\build\vs2015 || goto error
 devenv genie.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto geniedone
)
if exist "C:\Program Files\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
 call build\auto\setup_vs2015.cmd || goto error
 cd include\genie\build\vs2015 || goto error
 devenv genie.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto geniedone
)

:geniedone



rem call :download_and_unpack "premake" "https://github.com/premake/premake-core/archive/2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6" "-" || goto error
call :unpack "premake" "premake\premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6.zip" "premake-core-2e7ca5fb18acdbcd5755fb741710622b20f2e0f6" "-" || goto error

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
 call build\auto\setup_vs2017.cmd || goto error
 cd include\premake || goto error
  nmake -f Bootstrap.mak windows MSDEV=vs2017 || goto error
  bin\release\premake5 --file=../../build/premake/fixup.lua fixup || goto error
  bin\release\premake5 embed --bytecode || goto error
  bin\release\premake5 --to=build/vs2017 vs2017 --no-curl --no-zlib --no-luasocket || goto error
 cd ..\.. || goto error
 cd include\premake\build\vs2017 || goto error
  devenv Premake5.sln /clean "Release|Win32" || goto error
  devenv Premake5.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto premakedone
)
if exist "C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
 call build\auto\setup_vs2017.cmd || goto error
 cd include\premake || goto error
  nmake -f Bootstrap.mak windows MSDEV=vs2017 || goto error
  bin\release\premake5 --file=../../build/premake/fixup.lua fixup || goto error
  bin\release\premake5 embed --bytecode || goto error
  bin\release\premake5 --to=build/vs2017 vs2017 --no-curl --no-zlib --no-luasocket || goto error
 cd ..\.. || goto error
 cd include\premake\build\vs2017 || goto error
  devenv Premake5.sln /clean "Release|Win32" || goto error
  devenv Premake5.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto premakedone
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
 call build\auto\setup_vs2015.cmd || goto error
 cd include\premake || goto error
  nmake -f Bootstrap.mak windows MSDEV=vs2015 || goto error
  bin\release\premake5 embed --bytecode || goto error
  bin\release\premake5 --to=build/vs2015 vs2015 --no-curl --no-zlib --no-luasocket || goto error
 cd ..\.. || goto error
 cd include\premake\build\vs2015 || goto error
  devenv Premake5.sln /clean "Release|Win32" || goto error
  devenv Premake5.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto premakedone
)
if exist "C:\Program Files\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
 call build\auto\setup_vs2015.cmd || goto error
 cd include\premake || goto error
  nmake -f Bootstrap.mak windows MSDEV=vs2015 || goto error
  bin\release\premake5 embed --bytecode || goto error
  bin\release\premake5 --to=build/vs2015 vs2015 --no-curl --no-zlib --no-luasocket || goto error
 cd ..\.. || goto error
 cd include\premake\build\vs2015 || goto error
  devenv Premake5.sln /clean "Release|Win32" || goto error
  devenv Premake5.sln /build "Release|Win32" || goto error
 cd ..\..\..\.. || goto error
 goto premakedone
)

goto error

:premakedone

goto ok

:ok
echo "All OK."
if "x%1" == "xauto" (
	exit 0
)
goto end
:error
echo "Error!"
if "x%1" == "xauto" (
	exit 1
)
goto end
:end
cd %MY_DIR%
pause
