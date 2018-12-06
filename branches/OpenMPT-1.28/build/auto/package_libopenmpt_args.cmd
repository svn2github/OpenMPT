@echo off

set BATCH_DIR=%~dp0
cd %BATCH_DIR%
cd ..\..

set MY_DIR=%CD%

call "build\auto\setup_arguments.cmd" %1 %2 %3 %4 %5 %6

call build\auto\setup_vs_any.cmd

call build\auto\helper_get_svnversion.cmd
call build\auto\helper_get_openmpt_version.cmd

set MPT_REVISION=%LIBOPENMPT_VERSION_STRING%+%SVNVERSION%
if "x%LIBOPENMPT_VERSION_PREREL%" == "x" (
	set MPT_REVISION=%LIBOPENMPT_VERSION_STRING%+release
)


set MPT_PKG_TAG=win%MPT_DIST_VARIANT_SUFFIX%



cd bin || goto error
rmdir /s /q libopenmpt
mkdir libopenmpt || goto error
mkdir libopenmpt\bin.%MPT_PKG_TAG%
mkdir libopenmpt\bin.%MPT_PKG_TAG%\%LIBOPENMPT_VERSION_STRING%
rmdir /s /q libopenmpt-%MPT_PKG_TAG%
del /f /q libopenmpt-%MPT_PKG_TAG%.tar
del /f /q libopenmpt-%MPT_REVISION%.bin.%MPT_PKG_TAG%.%MPT_PKG_FORMAT%
mkdir libopenmpt-%MPT_PKG_TAG%
cd libopenmpt-%MPT_PKG_TAG% || goto error
mkdir openmpt123
mkdir openmpt123\x86
mkdir openmpt123\x86_64
mkdir XMPlay
mkdir Winamp
mkdir foobar2000
rmdir /s /q Licenses
mkdir Licenses
copy /y ..\..\include\flac\COPYING.Xiph                      .\Licenses\License.FLAC.txt || goto error
copy /y ..\..\include\foobar2000sdk\sdk-license.txt          .\Licenses\License.Foobar2000SDK.txt || goto error
copy /y ..\..\include\foobar2000sdk\pfc\pfc-license.txt      .\Licenses\License.Foobar2000SDK-pfc.txt || goto error
rem copy /y ..\..\include\lame\COPYING                           .\Licenses\License.lame.txt || goto error
rem copy /y ..\..\include\lhasa\COPYING                          .\Licenses\License.lhasa.txt || goto error
rem copy /y ..\..\include\minimp3\LICENSE                        .\Licenses\License.minimp3.txt || goto error
rem copy /y ..\..\include\miniz\miniz.c                          .\Licenses\License.miniz.txt || goto error
copy /y ..\..\include\mpg123\COPYING                         .\Licenses\License.mpg123.txt || goto error
copy /y ..\..\include\mpg123\AUTHORS                         .\Licenses\License.mpg123.Authors.txt || goto error
rem copy /y ..\include\nlohmann-json\LICENSE.MIT                 .\Licenses\License.nlohmann-json.txt || goto error
copy /y ..\..\include\ogg\COPYING                            .\Licenses\License.ogg.txt || goto error
rem copy /y ..\..\include\opus\COPYING                           .\Licenses\License.Opus.txt || goto error
rem copy /y ..\..\include\opusenc\COPYING                        .\Licenses\License.Opusenc.txt || goto error
rem copy /y ..\..\include\opusenc\AUTHORS                        .\Licenses\License.Opusenc.Authors.txt || goto error
rem copy /y ..\..\include\opusfile\COPYING                       .\Licenses\License.Opusfile.txt || goto error
copy /y ..\..\include\portaudio\LICENSE.txt                  .\Licenses\License.PortAudio.txt || goto error
copy /y ..\..\include\portaudio\bindings\cpp\COPYING         .\Licenses\License.portaudiocpp.txt || goto error
copy /y ..\..\include\pugixml\readme.txt                     .\Licenses\License.PugiXML.txt || goto error
rem copy /y ..\..\include\r8brain\other\License.txt              .\Licenses\License.R8Brain.txt || goto error
rem copy /y ..\..\include\rtaudio\readme                         .\Licenses\License.RtAudio.txt || goto error
rem copy /y ..\..\include\rtmidi\License.txt                     .\Licenses\License.RtMidi.txt || goto error
rem copy /y ..\..\include\smbPitchShift\smbPitchShift.cpp        .\Licenses\License.smbPitchShift.txt || goto error
rem copy /y ..\..\include\soundtouch\COPYING.TXT                 .\Licenses\License.SoundTouch.txt || goto error
rem copy /y ..\..\include\stb_vorbis\stb_vorbis.c                .\Licenses\License.stb_vorbis.txt || goto error
rem copy /y ..\..\include\unrar\license.txt                      .\Licenses\License.UnRAR.txt || goto error
copy /y ..\..\include\vorbis\COPYING                         .\Licenses\License.Vorbis.txt || goto error
copy /y ..\..\include\zlib\README                            .\Licenses\License.zlib.txt || goto error
rem copy /y ..\..\include\zlib\contrib\minizip\MiniZip64_info.txt .\Licenses\License.minizip.txt || goto error
copy /y ..\..\LICENSE .\LICENSE.txt || goto error
copy /y ..\..\libopenmpt\dox\changelog.md .\ || goto error
copy /y ..\..\libopenmpt\doc\xmp-openmpt.txt .\XMPlay\ || goto error
copy /y ..\..\libopenmpt\doc\in_openmpt.txt .\Winamp\ || goto error
copy /y ..\..\libopenmpt\doc\foo_openmpt.txt .\foobar2000\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\openmpt123.exe .\openmpt123\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\openmpt-mpg123.dll .\openmpt123\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-64-%MPT_BIN_TARGET64%\openmpt123.exe .\openmpt123\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-64-%MPT_BIN_TARGET64%\openmpt-mpg123.dll .\openmpt123\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\xmp-openmpt.dll .\XMPlay\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\openmpt-mpg123.dll .\XMPlay\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\in_openmpt.dll .\Winamp\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\openmpt-mpg123.dll .\Winamp\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-static\x86-32-%MPT_BIN_TARGET32%\foo_openmpt.fb2k-component .\foobar2000\ || goto error
..\..\build\tools\7zip\7z.exe a -t%MPT_PKG_FORMAT% -mx=9 ..\libopenmpt\bin.%MPT_PKG_TAG%\%LIBOPENMPT_VERSION_STRING%\libopenmpt-%MPT_REVISION%.bin.%MPT_PKG_TAG%.%MPT_PKG_FORMAT% ^
 LICENSE.txt ^
 Licenses ^
 changelog.md ^
 openmpt123\x86\openmpt123.exe ^
 openmpt123\x86\openmpt-mpg123.dll ^
 openmpt123\x86_64\openmpt123.exe ^
 openmpt123\x86_64\openmpt-mpg123.dll ^
 XMPlay\xmp-openmpt.txt ^
 XMPlay\xmp-openmpt.dll ^
 XMPlay\openmpt-mpg123.dll ^
 Winamp\in_openmpt.txt ^
 Winamp\in_openmpt.dll ^
 Winamp\openmpt-mpg123.dll ^
 foobar2000\foo_openmpt.txt ^
 foobar2000\foo_openmpt.fb2k-component ^
 || goto error
cd .. || goto error
rmdir /s /q libopenmpt-%MPT_PKG_TAG%
..\build\tools\7zip\7z.exe a -r -ttar libopenmpt-%MPT_PKG_TAG%.tar libopenmpt || goto error
del /f /q libopenmpt\bin.%MPT_PKG_TAG%\%LIBOPENMPT_VERSION_STRING%\libopenmpt-%MPT_REVISION%.bin.%MPT_PKG_TAG%.%MPT_PKG_FORMAT%
rmdir /s /q libopenmpt
cd .. || goto error

cd bin || goto error
rmdir /s /q libopenmpt
mkdir libopenmpt || goto error
mkdir libopenmpt\dev.%MPT_PKG_TAG%.%MPT_VS_VER%
mkdir libopenmpt\dev.%MPT_PKG_TAG%.%MPT_VS_VER%\%LIBOPENMPT_VERSION_STRING%
rmdir /s /q libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER%
del /f /q libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER%.tar
del /f /q libopenmpt-%MPT_REVISION%.dev.%MPT_PKG_TAG%.%MPT_VS_VER%.%MPT_PKG_FORMAT%
mkdir libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER%
cd libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER% || goto error
mkdir inc
mkdir inc\libopenmpt
mkdir lib
mkdir lib\x86
mkdir lib\x86_64
mkdir bin
mkdir bin\x86
mkdir bin\x86_64
copy /y ..\..\LICENSE .\LICENSE.txt || goto error
rmdir /s /q Licenses
mkdir Licenses
rem copy /y ..\..\include\flac\COPYING.Xiph                      .\Licenses\License.FLAC.txt || goto error
rem copy /y ..\..\include\foobar2000sdk\sdk-license.txt          .\Licenses\License.Foobar2000SDK.txt || goto error
rem copy /y ..\..\include\foobar2000sdk\pfc\pfc-license.txt      .\Licenses\License.Foobar2000SDK-pfc.txt || goto error
rem copy /y ..\..\include\lame\COPYING                           .\Licenses\License.lame.txt || goto error
rem copy /y ..\..\include\lhasa\COPYING                          .\Licenses\License.lhasa.txt || goto error
rem copy /y ..\..\include\minimp3\LICENSE                        .\Licenses\License.minimp3.txt || goto error
rem copy /y ..\..\include\miniz\miniz.c                          .\Licenses\License.miniz.txt || goto error
copy /y ..\..\include\mpg123\COPYING                         .\Licenses\License.mpg123.txt || goto error
copy /y ..\..\include\mpg123\AUTHORS                         .\Licenses\License.mpg123.Authors.txt || goto error
rem copy /y ..\include\nlohmann-json\LICENSE.MIT                 .\Licenses\License.nlohmann-json.txt || goto error
copy /y ..\..\include\ogg\COPYING                            .\Licenses\License.ogg.txt || goto error
rem copy /y ..\..\include\opus\COPYING                           .\Licenses\License.Opus.txt || goto error
rem copy /y ..\..\include\opusenc\COPYING                        .\Licenses\License.Opusenc.txt || goto error
rem copy /y ..\..\include\opusenc\AUTHORS                        .\Licenses\License.Opusenc.Authors.txt || goto error
rem copy /y ..\..\include\opusfile\COPYING                       .\Licenses\License.Opusfile.txt || goto error
rem copy /y ..\..\include\portaudio\LICENSE.txt                  .\Licenses\License.PortAudio.txt || goto error
rem copy /y ..\..\include\portaudio\bindings\cpp\COPYING         .\Licenses\License.portaudiocpp.txt || goto error
rem copy /y ..\..\include\pugixml\readme.txt                     .\Licenses\License.PugiXML.txt || goto error
rem copy /y ..\..\include\r8brain\other\License.txt              .\Licenses\License.R8Brain.txt || goto error
rem copy /y ..\..\include\rtaudio\readme                         .\Licenses\License.RtAudio.txt || goto error
rem copy /y ..\..\include\rtmidi\License.txt                     .\Licenses\License.RtMidi.txt || goto error
rem copy /y ..\..\include\smbPitchShift\smbPitchShift.cpp        .\Licenses\License.smbPitchShift.txt || goto error
rem copy /y ..\..\include\soundtouch\COPYING.TXT                 .\Licenses\License.SoundTouch.txt || goto error
rem copy /y ..\..\include\stb_vorbis\stb_vorbis.c                .\Licenses\License.stb_vorbis.txt || goto error
rem copy /y ..\..\include\unrar\license.txt                      .\Licenses\License.UnRAR.txt || goto error
copy /y ..\..\include\vorbis\COPYING                         .\Licenses\License.Vorbis.txt || goto error
copy /y ..\..\include\zlib\README                            .\Licenses\License.zlib.txt || goto error
rem copy /y ..\..\include\zlib\contrib\minizip\MiniZip64_info.txt .\Licenses\License.minizip.txt || goto error
copy /y ..\..\libopenmpt\dox\changelog.md .\changelog.md || goto error
copy /y ..\..\libopenmpt\libopenmpt.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt.hpp inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_config.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_version.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_ext.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_ext.hpp inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_stream_callbacks_buffer.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_stream_callbacks_fd.h inc\libopenmpt\ || goto error
copy /y ..\..\libopenmpt\libopenmpt_stream_callbacks_file.h inc\libopenmpt\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\libopenmpt.lib lib\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\libopenmpt.dll bin\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\openmpt-mpg123.dll bin\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\openmpt-ogg.dll bin\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\openmpt-vorbis.dll bin\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-32-%MPT_BIN_TARGET32%\openmpt-zlib.dll bin\x86\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\libopenmpt.lib lib\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\libopenmpt.dll bin\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\openmpt-mpg123.dll bin\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\openmpt-ogg.dll bin\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\openmpt-vorbis.dll bin\x86_64\ || goto error
copy /y ..\..\bin\release\%MPT_VS_VER%-shared\x86-64-%MPT_BIN_TARGET64%\openmpt-zlib.dll bin\x86_64\ || goto error
..\..\build\tools\7zip\7z.exe a -t%MPT_PKG_FORMAT% -mx=9 ..\libopenmpt\dev.%MPT_PKG_TAG%.%MPT_VS_VER%\%LIBOPENMPT_VERSION_STRING%\libopenmpt-%MPT_REVISION%.dev.%MPT_PKG_TAG%.%MPT_VS_VER%.%MPT_PKG_FORMAT% ^
 LICENSE.txt ^
 Licenses ^
 changelog.md ^
 inc\libopenmpt\libopenmpt.h ^
 inc\libopenmpt\libopenmpt.hpp ^
 inc\libopenmpt\libopenmpt_config.h ^
 inc\libopenmpt\libopenmpt_version.h ^
 inc\libopenmpt\libopenmpt_ext.h ^
 inc\libopenmpt\libopenmpt_ext.hpp ^
 inc\libopenmpt\libopenmpt_stream_callbacks_buffer.h ^
 inc\libopenmpt\libopenmpt_stream_callbacks_fd.h ^
 inc\libopenmpt\libopenmpt_stream_callbacks_file.h ^
 lib\x86\libopenmpt.lib ^
 lib\x86_64\libopenmpt.lib ^
 bin\x86\libopenmpt.dll ^
 bin\x86\openmpt-mpg123.dll ^
 bin\x86\openmpt-ogg.dll ^
 bin\x86\openmpt-vorbis.dll ^
 bin\x86\openmpt-zlib.dll ^
 bin\x86_64\libopenmpt.dll ^
 bin\x86_64\openmpt-mpg123.dll ^
 bin\x86_64\openmpt-ogg.dll ^
 bin\x86_64\openmpt-vorbis.dll ^
 bin\x86_64\openmpt-zlib.dll ^
 || goto error
cd .. || goto error
rmdir /s /q libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER%
..\build\tools\7zip\7z.exe a -r -ttar libopenmpt-dev-%MPT_PKG_TAG%-%MPT_VS_VER%.tar libopenmpt || goto error
del /f /q libopenmpt\dev.%MPT_PKG_TAG%.%MPT_VS_VER%\%LIBOPENMPT_VERSION_STRING%\libopenmpt-%MPT_REVISION%.dev.%MPT_PKG_TAG%.%MPT_VS_VER%.%MPT_PKG_FORMAT%
rmdir /s /q libopenmpt
cd .. || goto error



goto noerror

:error
cd "%MY_DIR%"
exit 1

:noerror
cd "%MY_DIR%"
exit 0
