@echo off

set MPT_VS_VER=%1
set MPT_VS_TARGET=%2
set MPT_VS_ARCH=%3
set MPT_VS_CONF=%4
set MPT_PKG_FORMAT=%5
set MPT_VS_FLAVOUR=%6

if "%MPT_VS_VER%" == "" goto setupargumentserror
if "%MPT_VS_TARGET%" == "" goto setupargumentserror
if "%MPT_VS_ARCH%" == "" goto setupargumentserror
if "%MPT_VS_CONF%" == "" goto setupargumentserror
if "%MPT_PKG_FORMAT%" == "" goto setupargumentserror

goto setupargumentsstart

:setupargumentserror
echo "Usage: foo.cmd vs2010 xp Win32 Release 7z default"
rem vs2010     xp            Win32       Release     x86-32       winxp          release      static
rem MPT_VS_VER MPT_VS_TARGET MPT_VS_ARCH MPT_VS_CONF MPT_BIN_ARCH MPT_BIN_TARGET MPT_BIN_CONF MPT_BIN_RUNTIME
exit 1

:setupargumentsstart


if "%MPT_VS_TARGET%" == "xp"      set MPT_VS_WITHTARGET=%MPT_VS_VER%xp
if "%MPT_VS_TARGET%" == "vista"   set MPT_VS_WITHTARGET=%MPT_VS_VER%
if "%MPT_VS_TARGET%" == "win10"   set MPT_VS_WITHTARGET=%MPT_VS_VER%win10
if "%MPT_VS_TARGET%" == "default" set MPT_VS_WITHTARGET=%MPT_VS_VER%

if "%MPT_VS_ARCH%" == "Win32" set MPT_VS_ARCH_OTHER=x64
if "%MPT_VS_ARCH%" == "x64"   set MPT_VS_ARCH_OTHER=Win32
if "%MPT_VS_ARCH%" == "ARM"   set MPT_VS_ARCH_OTHER=Win32
if "%MPT_VS_ARCH%" == "ARM64" set MPT_VS_ARCH_OTHER=Win32


if "%MPT_VS_ARCH%" == "Win32" (
	if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET=winxp
	if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET=vista
	if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET=win10
	if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET=win7
)
if "%MPT_VS_ARCH%" == "x64" (
	if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET=winxp64
	if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET=vista
	if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET=win10
	if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET=win7
)
if "%MPT_VS_ARCH%" == "ARM" (
	if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET=winxp
	if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET=vista
	if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET=win10
	if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET=win7
)
if "%MPT_VS_ARCH%" == "ARM64" (
	if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET=winxp
	if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET=vista
	if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET=win10
	if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET=win7
)

if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET32=winxp
if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET32=vista
if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET32=win10
if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET32=win7

if "%MPT_VS_TARGET%" == "xp"      set MPT_BIN_TARGET64=winxp64
if "%MPT_VS_TARGET%" == "vista"   set MPT_BIN_TARGET64=vista
if "%MPT_VS_TARGET%" == "win10"   set MPT_BIN_TARGET64=win10
if "%MPT_VS_TARGET%" == "default" set MPT_BIN_TARGET64=win7

if "%MPT_VS_ARCH%" == "Win32" set MPT_BIN_ARCH=x86-32
if "%MPT_VS_ARCH%" == "x64"   set MPT_BIN_ARCH=x86-64
if "%MPT_VS_ARCH%" == "ARM"   set MPT_BIN_ARCH=arm-32
if "%MPT_VS_ARCH%" == "ARM64" set MPT_BIN_ARCH=arm-64

if "%MPT_VS_CONF%" == "Release"       set MPT_BIN_CONF=release
if "%MPT_VS_CONF%" == "ReleaseShared" set MPT_BIN_CONF=release
if "%MPT_VS_CONF%" == "ReleaseLTCG"   set MPT_BIN_CONF=release-LTCG

if "%MPT_VS_CONF%" == "Release"       set MPT_BIN_RUNTIME=static
if "%MPT_VS_CONF%" == "ReleaseShared" set MPT_BIN_RUNTIME=shared
if "%MPT_VS_CONF%" == "ReleaseLTCG"   set MPT_BIN_RUNTIME=static


if "%MPT_VS_ARCH%" == "Win32" set MPT_DIST_VARIANT_PREFIX=win32
if "%MPT_VS_ARCH%" == "x64"   set MPT_DIST_VARIANT_PREFIX=win64
if "%MPT_VS_ARCH%" == "ARM"   set MPT_DIST_VARIANT_PREFIX=arm32
if "%MPT_VS_ARCH%" == "ARM64" set MPT_DIST_VARIANT_PREFIX=arm64

if "%MPT_VS_TARGET%" == "xp"      set MPT_DIST_VARIANT_SUFFIX=old
if "%MPT_VS_TARGET%" == "vista"   set MPT_DIST_VARIANT_SUFFIX=old
if "%MPT_VS_TARGET%" == "win10"   set MPT_DIST_VARIANT_SUFFIX=win10
if "%MPT_VS_TARGET%" == "default" set MPT_DIST_VARIANT_SUFFIX=

set MPT_DIST_VARIANT=%MPT_DIST_VARIANT_PREFIX%%MPT_DIST_VARIANT_SUFFIX%


rem Environment Variable \ Program Bitness 32bit Native 64bit Native WOW64
rem PROCESSOR_ARCHITECTURE                 x86          AMD64        x86
rem PROCESSOR_ARCHITEW6432                 undefined    undefined    AMD64

set MPT_HOST_BITNESS=32
if "%PROCESSOR_ARCHITECTURE%" == "x86" (
	if "%PROCESSOR_ARCHITEW6432%" == "AMD64" (
		set MPT_HOST_BITNESS=64
	) else (
		set MPT_HOST_BITNESS=32
	)
)
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
	set MPT_HOST_BITNESS=64
)


if "%MPT_PKG_FORMAT%" == "" set MPT_PKG_FORMAT=zip
if "%MPT_PKG_FORMAT%" == "7z" set MPT_PKG_FORMAT_SYMBOLS=xz
if "%MPT_PKG_FORMAT%" == "zip" set MPT_PKG_FORMAT_SYMBOLS=zip
if "%MPT_PKG_FORMAT_SYMBOLS%" == "" set MPT_PKG_FORMAT_SYMBOLS=zip


if "%MPT_VS_FLAVOUR%" == "default" set MPT_VS_FLAVOUR=
