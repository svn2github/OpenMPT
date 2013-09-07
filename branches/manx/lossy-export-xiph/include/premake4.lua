solution "include"
 configurations { "DebugLib", "NormalLib", "ReleaseLib" }
 platforms { "x32", "x64" }
 
 
 
 project "zlib"
  uuid "1654FB18-FDE6-406F-9D84-BA12BFBD67B2"
  language "C"
  location "zlib"
  includedirs { "zlib" }
  files {
   "zlib/adler32.c",
   "zlib/compress.c",
   "zlib/crc32.c",
   "zlib/deflate.c",
   "zlib/gzclose.c",
   "zlib/gzlib.c",
   "zlib/gzread.c",
   "zlib/gzwrite.c",
   "zlib/infback.c",
   "zlib/inffast.c",
   "zlib/inflate.c",
   "zlib/inftrees.c",
   "zlib/trees.c",
   "zlib/uncompr.c",
   "zlib/zutil.c",
  }
  files {
   "zlib/crc32.h",
   "zlib/deflate.h",
   "zlib/gzguts.h",
   "zlib/inffast.h",
   "zlib/inffixed.h",
   "zlib/inflate.h",
   "zlib/inftrees.h",
   "zlib/trees.h",
   "zlib/zconf.h",
   "zlib/zlib.h",
   "zlib/zutil.h",
  }
  configuration "*DLL"
   defines { "ZLIB_DLL" }
  dofile "premake4-defaults.lua"
  
  
  
 project "minizip"
  uuid "63AF9025-A6CE-4147-A05D-6E2CCFD3A0D7"
  language "C"
  location "zlib"
  includedirs { "zlib", "zlib/contrib/minizip" }
  files {
   "zlib/contrib/minizip/ioapi.c",
   "zlib/contrib/minizip/iowin32.c",
   "zlib/contrib/minizip/mztools.c",
   "zlib/contrib/minizip/unzip.c",
   "zlib/contrib/minizip/zip.c",
  }
  files {
   "zlib/contrib/minizip/crypt.h",
   "zlib/contrib/minizip/ioapi.h",
   "zlib/contrib/minizip/iowin32.h",
   "zlib/contrib/minizip/mztools.h",
   "zlib/contrib/minizip/unzip.h",
   "zlib/contrib/minizip/zip.h",
  }
  links { "zlib" }
  configuration "*DLL"
   defines { "ZLIB_DLL" }
  dofile "premake4-defaults.lua"
   
   
   
 project "smbPitchShift"
  uuid "89AF16DD-32CC-4A7E-B219-5F117D761F9F"
  language "C++"
  location "smbPitchShift"
  includedirs { }
  files {
   "smbPitchShift/smbPitchShift.cpp",
  }
  files {
   "smbPitchShift/smbPitchShift.h",
  }
  dofile "premake4-defaults.lua"

  
  
 project "lhasa"
  uuid "6B11F6A8-B131-4D2B-80EF-5731A9016436"
  language "C"
  location "lhasa"
  includedirs { "msinttypes/inttypes" }
  files {
   "lhasa/lib/crc16.c",
   "lhasa/lib/ext_header.c",
   "lhasa/lib/lh1_decoder.c",
   "lhasa/lib/lh5_decoder.c",
   "lhasa/lib/lh6_decoder.c",
   "lhasa/lib/lh7_decoder.c",
   "lhasa/lib/lha_arch_unix.c",
   "lhasa/lib/lha_arch_win32.c",
   "lhasa/lib/lha_basic_reader.c",
   "lhasa/lib/lha_decoder.c",
   "lhasa/lib/lha_endian.c",
   "lhasa/lib/lha_file_header.c",
   "lhasa/lib/lha_input_stream.c",
   "lhasa/lib/lha_reader.c",
   "lhasa/lib/lz5_decoder.c",
   "lhasa/lib/lzs_decoder.c",
   "lhasa/lib/macbinary.c",
   "lhasa/lib/null_decoder.c",
   "lhasa/lib/pm1_decoder.c",
   "lhasa/lib/pm2_decoder.c",
  }
  files {
   "lhasa/lib/crc16.h",
   "lhasa/lib/ext_header.h",
   "lhasa/lib/lha_arch.h",
   "lhasa/lib/lha_basic_reader.h",
   "lhasa/lib/lha_decoder.h",
   "lhasa/lib/lha_endian.h",
   "lhasa/lib/lha_file_header.h",
   "lhasa/lib/lha_input_stream.h",
   "lhasa/lib/macbinary.h",
   "lhasa/lib/public/lha_decoder.h",
   "lhasa/lib/public/lha_file_header.h",
   "lhasa/lib/public/lha_input_stream.h",
   "lhasa/lib/public/lha_reader.h",
   "lhasa/lib/public/lhasa.h",
  }
  configuration "vs2008"
   includedirs { "msinttypes/stdint" }
  dofile "premake4-defaults.lua"


    
 project "flac"
  uuid "E599F5AA-F9A3-46CC-8DB0-C8DEFCEB90C5"
  language "C"
  location "flac"
  includedirs { "flac/include", "flac/src/libFLAC/include" }
  files {
   "flac/src/libFLAC/bitmath.c",
   "flac/src/libFLAC/bitreader.c",
   "flac/src/libFLAC/bitwriter.c",
   "flac/src/libFLAC/cpu.c",
   "flac/src/libFLAC/crc.c",
   "flac/src/libFLAC/fixed.c",
   "flac/src/libFLAC/float.c",
   "flac/src/libFLAC/format.c",
   "flac/src/libFLAC/lpc.c",
   "flac/src/libFLAC/md5.c",
   "flac/src/libFLAC/memory.c",
   "flac/src/libFLAC/metadata_iterators.c",
   "flac/src/libFLAC/metadata_object.c",
   "flac/src/libFLAC/stream_decoder.c",
   "flac/src/libFLAC/stream_encoder.c",
   "flac/src/libFLAC/stream_encoder_framing.c",
   "flac/src/libFLAC/window.c",
  }
  files {
   "flac/src/libFLAC/include/private/all.h",
   "flac/src/libFLAC/include/private/bitmath.h",
   "flac/src/libFLAC/include/private/bitreader.h",
   "flac/src/libFLAC/include/private/bitwriter.h",
   "flac/src/libFLAC/include/private/cpu.h",
   "flac/src/libFLAC/include/private/crc.h",
   "flac/src/libFLAC/include/private/fixed.h",
   "flac/src/libFLAC/include/private/float.h",
   "flac/src/libFLAC/include/private/format.h",
   "flac/src/libFLAC/include/private/lpc.h",
   "flac/src/libFLAC/include/private/md5.h",
   "flac/src/libFLAC/include/private/memory.h",
   "flac/src/libFLAC/include/private/metadata.h",
   "flac/src/libFLAC/include/private/stream_encoder_framing.h",
   "flac/src/libFLAC/include/private/window.h",
   "flac/src/libFLAC/include/protected/all.h",
   "flac/src/libFLAC/include/protected/stream_decoder.h",
   "flac/src/libFLAC/include/protected/stream_encoder.h",
  }
  files {
   "flac/include/FLAC/all.h",
   "flac/include/FLAC/assert.h",
   "flac/include/FLAC/callback.h",
   "flac/include/FLAC/export.h",
   "flac/include/FLAC/format.h",
   "flac/include/FLAC/metadata.h",
   "flac/include/FLAC/ordinals.h",
   "flac/include/FLAC/stream_decoder.h",
   "flac/include/FLAC/stream_encoder.h",
  }
  configuration "*Lib"
   defines { "FLAC__NO_DLL" }
  configuration "vs2010"
   defines { "VERSION=\"1.2.1\"" }
  configuration "vs2008"
   defines { "VERSION=\\\"1.2.1\\\"" }
  dofile "premake4-defaults.lua"

  
  
 project "portaudio"
  uuid "189B867F-FF4B-45A1-B741-A97492EE69AF"
  language "C"
  location "portaudio"
  includedirs { "portaudio/include", "portaudio/src/common", "portaudio/src/os/win" }
  defines {
   "PA_ENABLE_DEBUG_OUTPUT",
   "PAWIN_USE_WDMKS_DEVICE_INFO",
   "PA_USE_ASIO=0",
   "PA_USE_DS=0",
   "PA_USE_WMME=1",
   "PA_USE_WASAPI=1",
   "PA_USE_WDMKS=1",
  }
  files {
   "portaudio/src/common/pa_allocation.c",
   "portaudio/src/common/pa_allocation.h",
   "portaudio/src/common/pa_converters.c",
   "portaudio/src/common/pa_converters.h",
   "portaudio/src/common/pa_cpuload.c",
   "portaudio/src/common/pa_cpuload.h",
   "portaudio/src/common/pa_debugprint.c",
   "portaudio/src/common/pa_debugprint.h",
   "portaudio/src/common/pa_dither.c",
   "portaudio/src/common/pa_dither.h",
   "portaudio/src/common/pa_endianness.h",
   "portaudio/src/common/pa_front.c",
   "portaudio/src/common/pa_hostapi.h",
   "portaudio/src/common/pa_memorybarrier.h",
   "portaudio/src/common/pa_process.c",
   "portaudio/src/common/pa_process.h",
   "portaudio/src/common/pa_ringbuffer.c",
   "portaudio/src/common/pa_ringbuffer.h",
   "portaudio/src/common/pa_stream.c",
   "portaudio/src/common/pa_stream.h",
   "portaudio/src/common/pa_trace.c",
   "portaudio/src/common/pa_trace.h",
   "portaudio/src/common/pa_types.h",
   "portaudio/src/common/pa_util.h",
   "portaudio/src/hostapi/skeleton/pa_hostapi_skeleton.c",
   "portaudio/src/hostapi/wasapi/pa_win_wasapi.c",
   "portaudio/src/hostapi/wdmks/pa_win_wdmks.c",
   "portaudio/src/hostapi/wmme/pa_win_wmme.c",
   "portaudio/src/os/win/pa_win_coinitialize.c",
   "portaudio/src/os/win/pa_win_coinitialize.h",
   "portaudio/src/os/win/pa_win_hostapis.c",
   "portaudio/src/os/win/pa_win_util.c",
   "portaudio/src/os/win/pa_win_waveformat.c",
   "portaudio/src/os/win/pa_win_wdmks_utils.c",
   "portaudio/src/os/win/pa_win_wdmks_utils.h",
   "portaudio/src/os/win/pa_x86_plain_converters.c",
   "portaudio/src/os/win/pa_x86_plain_converters.h",
  }
  files {
   "portaudio/include/pa_asio.h",
   "portaudio/include/pa_jack.h",
   "portaudio/include/pa_linux_alsa.h",
   "portaudio/include/pa_mac_core.h",
   "portaudio/include/pa_win_ds.h",
   "portaudio/include/pa_win_wasapi.h",
   "portaudio/include/pa_win_waveformat.h",
   "portaudio/include/pa_win_wmme.h",
   "portaudio/include/portaudio.h",
  }
  buildoptions { "/wd4018" }
  dofile "premake4-defaults.lua"
  

  
 project "portmidi"
  uuid "2512E2CA-578A-4F10-9363-4BFC9A5EF960"
  language "C"
  location "portmidi"
  includedirs { "portmidi/porttime", "portmidi/pm_common", "portmidi/pm_win" }
  files {
   "portmidi/porttime/porttime.c",
   "portmidi/porttime/ptwinmm.c",
   "portmidi/pm_common/pmutil.c",
   "portmidi/pm_common/portmidi.c",
   "portmidi/pm_win/pmwin.c",
   "portmidi/pm_win/pmwinmm.c",
  }
  files {
   "portmidi/porttime/porttime.h",
   "portmidi/pm_common/pminternal.h",
   "portmidi/pm_common/pmutil.h",
   "portmidi/pm_common/portmidi.h",
   "portmidi/pm_win/pmwinmm.h",
  }
  dofile "premake4-defaults.lua"
  

  
 project "ogg"
  uuid "AF40FE42-9999-4625-AD51-B1CEE39C1796"
  language "C"
  location "ogg"
  includedirs { "ogg/include" }
  files {
   "ogg/src/bitwise.c",
   "ogg/src/framing.c",
  }
  files {
   "ogg/include/ogg/ogg.h",
   "ogg/include/ogg/os_types.h",
  }
  dofile "premake4-defaults.lua"
  
 project "vorbis"
  uuid "EF71FEC3-22EC-4FB0-AC76-59ED642FDE61"
  language "C"
  location "vorbis"
  includedirs { "vorbis/include" }
  includedirs { "ogg/include" }
  files {
   "vorbis/lib/analysis.c",
   "vorbis/lib/bitrate.c",
   "vorbis/lib/block.c",
   "vorbis/lib/codebook.c",
   "vorbis/lib/envelope.c",
   "vorbis/lib/floor0.c",
   "vorbis/lib/floor1.c",
   "vorbis/lib/info.c",
   "vorbis/lib/lookup.c",
   "vorbis/lib/lpc.c",
   "vorbis/lib/lsp.c",
   "vorbis/lib/mapping0.c",
   "vorbis/lib/mdct.c",
   "vorbis/lib/psy.c",
   "vorbis/lib/registry.c",
   "vorbis/lib/res0.c",
   "vorbis/lib/sharedbook.c",
   "vorbis/lib/smallft.c",
   "vorbis/lib/synthesis.c",
   "vorbis/lib/vorbisenc.c",
   "vorbis/lib/window.c",
   "vorbis/lib/vorbisenc.c",
   "vorbis/lib/vorbisfile.c",
  }
  files {
   "vorbis/include/vorbis/codec.h",
   "vorbis/include/vorbis/vorbisenc.h",
   "vorbis/include/vorbis/vorbisfile.h",
  }
  buildoptions { "/wd4101", "/wd4244" }
  dofile "premake4-defaults.lua"

 project "opus"
  uuid "059F458F-BBAE-440F-B3AB-CF6788E4ACC2"
  language "C"
  location "opus"
  includedirs { "opus", "opus/include", "opus/src", "opus/celt", "opus/silk", "opus/silk/float", "opus/win32" }
  includedirs { "ogg/include" }
  files {
   "opus/src/analysis.c",
   "opus/src/analysis.h",
   "opus/src/mlp.c",
   "opus/src/mlp.h",
   "opus/src/mlp_data.c",
   "opus/src/opus.c",
   "opus/src/opus_decoder.c",
   "opus/src/opus_encoder.c",
   "opus/src/opus_multistream.c",
   "opus/src/opus_multistream_decoder.c",
   "opus/src/opus_multistream_encoder.c",
   "opus/src/repacketizer.c",
   "opus/celt/*.c",
   "opus/celt/*.h",
   "opus/silk/*.c",
   "opus/silk/*.h",
   "opus/silk/float/*.c",
   "opus/silk/float/*.h",
   "opus/win32/*.h",
  }
  files {
   "opus/include/opus.h",
   "opus/include/opus_custom.h",
   "opus/include/opus_defines.h",
   "opus/include/opus_multistream.h",
   "opus/include/opus_types.h",
  }
  defines { "HAVE_CONFIG_H" }
  buildoptions { "/wd4244" }
  dofile "premake4-defaults.lua"

  
  
  
solution "includeDLL"
 configurations { "DebugDLL", "NormalDLL", "ReleaseDLL" }
 platforms { "x32", "x64" }
 
 
 
 project "soundtouch"
  uuid "F5F8F6DE-84CF-4E9D-91EA-D9B5E2AA36CD"
  language "C++"
  location "soundtouch"
  targetname "OpenMPT_SoundTouch_i16"
  includedirs { }
  files {
   "soundtouch/AAFilter.cpp",
   "soundtouch/BPMDetect.cpp",
   "soundtouch/cpu_detect_x86_win.cpp",
   "soundtouch/FIFOSampleBuffer.cpp",
   "soundtouch/FIRFilter.cpp",
   "soundtouch/mmx_optimized.cpp",
   "soundtouch/PeakFinder.cpp",
   "soundtouch/RateTransposer.cpp",
   "soundtouch/SoundTouch.cpp",
   "soundtouch/SoundTouchDLL.cpp",
   "soundtouch/sse_optimized.cpp",
   "soundtouch/TDStretch.cpp",
  }
  files {
   "soundtouch/AAFilter.h",
   "soundtouch/BPMDetect.h",
   "soundtouch/cpu_detect.h",
   "soundtouch/FIFOSampleBuffer.h",
   "soundtouch/FIFOSamplePipe.h",
   "soundtouch/FIRFilter.h",
   "soundtouch/PeakFinder.h",
   "soundtouch/RateTransposer.h",
   "soundtouch/SoundTouch.h",
   "soundtouch/SoundTouchDLL.h",
   "soundtouch/STTypes.h",
   "soundtouch/TDStretch.h",
  }
  defines { "DLL_EXPORTS", "SOUNDTOUCH_INTEGER_SAMPLES=1" }
  dofile "premake4-defaults.lua"
 
 
 
 