@echo off

cd packageTemplate || goto error
copy /y ..\LICENSE .\License.txt || goto error
rmdir /s /q Licenses
mkdir Licenses
copy /y ..\include\flac\COPYING.Xiph                      .\Licenses\License.FLAC.txt || goto error
copy /y ..\include\lame\COPYING                           .\Licenses\License.lame.txt || goto error
copy /y ..\include\lhasa\COPYING                          .\Licenses\License.lhasa.txt || goto error
rem copy /y ..\..\include\minimp3\LICENSE                     .\Licenses\License.minimp3.txt || goto error
rem copy /y ..\include\miniz\miniz.c                          .\Licenses\License.miniz.txt || goto error
copy /y ..\include\mpg123\COPYING                         .\Licenses\License.mpg123.txt || goto error
copy /y ..\include\mpg123\AUTHORS                         .\Licenses\License.mpg123.Authors.txt || goto error
copy /y ..\include\nlohmann-json\LICENSE.MIT              .\Licenses\License.nlohmann-json.txt || goto error
copy /y ..\include\ogg\COPYING                            .\Licenses\License.ogg.txt || goto error
copy /y ..\include\opus\COPYING                           .\Licenses\License.Opus.txt || goto error
copy /y ..\include\opusenc\COPYING                        .\Licenses\License.Opusenc.txt || goto error
copy /y ..\include\opusenc\AUTHORS                        .\Licenses\License.Opusenc.Authors.txt || goto error
copy /y ..\include\opusfile\COPYING                       .\Licenses\License.Opusfile.txt || goto error
copy /y ..\include\portaudio\LICENSE.txt                  .\Licenses\License.PortAudio.txt || goto error
rem copy /y ..\include\portaudio\bindings\cpp\COPYING         .\Licenses\License.portaudiocpp.txt || goto error
rem copy /y ..\include\pugixml\readme.txt                     .\Licenses\License.PugiXML.txt || goto error
copy /y ..\include\r8brain\other\License.txt              .\Licenses\License.R8Brain.txt || goto error
copy /y ..\include\rtaudio\readme                         .\Licenses\License.RtAudio.txt || goto error
copy /y ..\include\rtkit\rtkit.h                          .\Licenses\License.RealtimeKit.txt || goto error
copy /y ..\include\rtmidi\License.txt                     .\Licenses\License.RtMidi.txt || goto error
copy /y ..\include\smbPitchShift\smbPitchShift.cpp        .\Licenses\License.smbPitchShift.txt || goto error
copy /y ..\include\soundtouch\COPYING.TXT                 .\Licenses\License.SoundTouch.txt || goto error
rem copy /y ..\include\stb_vorbis\stb_vorbis.c                .\Licenses\License.stb_vorbis.txt || goto error
copy /y ..\include\unrar\license.txt                      .\Licenses\License.UnRAR.txt || goto error
copy /y ..\include\vorbis\COPYING                         .\Licenses\License.Vorbis.txt || goto error
copy /y ..\include\zlib\README                            .\Licenses\License.zlib.txt || goto error
copy /y ..\include\zlib\contrib\minizip\MiniZip64_info.txt .\Licenses\License.minizip.txt || goto error
cd .. || goto error

goto noerror

:error
exit 1

:noerror
exit 0
