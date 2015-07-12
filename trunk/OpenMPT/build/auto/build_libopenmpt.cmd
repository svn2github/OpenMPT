@echo off

set BATCH_DIR=%~dp0
cd %BATCH_DIR%
cd ..\..

set MY_DIR=%CD%

set MPT_VSVER=%1
set MPT_ARCH=%2

if "%MPT_VSVER%" == "" goto usage
if "%MPT_ARCH%" == "" goto usage

call "build\auto\setup_%MPT_VSVER%.cmd"



cd "build\%MPT_VSVER%" || goto error

 devenv libopenmpt.sln /clean "ReleaseNoSSE2|%MPT_ARCH%" || goto error
 devenv openmpt123.sln /clean "ReleaseNoSSE2|%MPT_ARCH%" || goto error
 if "%MPT_ARCH%" == "Win32" (
  devenv in_openmpt.sln /clean "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  devenv xmp-openmpt.sln /clean "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  if "%MPT_VSVER%" == "vs2010" (
   devenv foo_openmpt.sln /clean "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  )
 )
 
 devenv libopenmpt.sln /build "ReleaseNoSSE2|%MPT_ARCH%" || goto error
 devenv openmpt123.sln /build "ReleaseNoSSE2|%MPT_ARCH%" || goto error
 if "%MPT_ARCH%" == "Win32" (
  devenv in_openmpt.sln /build "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  devenv xmp-openmpt.sln /build "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  if "%MPT_VSVER%" == "vs2010" (
   devenv foo_openmpt.sln /build "ReleaseNoSSE2|%MPT_ARCH%" || goto error
  )
 )

cd ..\.. || goto error



goto noerror

:usage
echo "Usage: foo.cmd vs2010 [Win32|x64]"
:error
cd "%MY_DIR%"
exit 1

:noerror
cd "%MY_DIR%"
exit 0
