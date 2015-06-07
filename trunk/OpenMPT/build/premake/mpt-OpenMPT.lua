
 project "OpenMPT"
  uuid "37FC32A4-8DDC-4A9C-A30C-62989DD8ACE9"
  language "C++"
  targetname "mptrack"
  location ( "../../build/" .. _ACTION )
  objdir "../../build/obj/OpenMPT"
  includedirs {
   "../../common",
   "../../soundlib",
   "../../include",
   "../../include/msinttypes/inttypes",
   "../../include/vstsdk2.4",
   "../../include/ASIOSDK2/common",
   "../../include/lhasa/lib/public",
   "../../include/ogg/include",
   "../../include/zlib",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../mptrack/res/OpenMPT.manifest",
  }
  files {
   "../../common/*.cpp",
   "../../common/*.h",
   "../../soundlib/*.cpp",
   "../../soundlib/*.h",
   "../../soundlib/plugins/*.cpp",
   "../../soundlib/plugins/*.h",
   "../../sounddsp/*.cpp",
   "../../sounddsp/*.h",
   "../../sounddev/*.cpp",
   "../../sounddev/*.h",
   "../../unarchiver/*.cpp",
   "../../unarchiver/*.h",
   "../../mptrack/*.cpp",
   "../../mptrack/*.h",
   "../../test/*.cpp",
   "../../test/*.h",
   "../../pluginBridge/BridgeCommon.h",
   "../../pluginBridge/BridgeWrapper.cpp",
   "../../pluginBridge/BridgeWrapper.h",
  }
  files {
   "../../mptrack/mptrack.rc",
   "../../mptrack/res/*.*", -- resource data files
  }
  excludes {
    "../../mptrack/res/rt_manif.bin", -- the old build system manifest
  }
	pchheader "stdafx.h"
	pchsource "../../common/stdafx.cpp"
  defines { "MODPLUG_TRACKER" }
  flags { "MFC", "SEH", "ExtraWarnings", "WinMain" }
  links {
   "UnRAR",
   "zlib",
   "minizip",
   "smbPitchShift",
   "lhasa",
   "flac",
   "ogg",
   "portaudio",
   "r8brain",
   "soundtouch",
  }
  linkoptions {
   "/DELAYLOAD:uxtheme.dll",
   "/DELAYLOAD:OpenMPT_SoundTouch_f32.dll",
  }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  dofile "../../build/premake/premake-defaults-EXEGUI.lua"
  dofile "../../build/premake/premake-defaults.lua"
  flags { "StaticRuntime" }



 project "VST MIDI Input Output"
  uuid "a181b7d5-54dd-42b6-9cc3-8ab8de2394be"
  language "C++"
  location ( "../../build/" .. _ACTION )
	targetname "MIDI Input Output"
  objdir "../../build/obj/PluginMidiInputOutput"
  includedirs {
   "../../include/vstsdk2.4",
   "../../include",
  }
  files {
   "../../plugins/MidiInOut/MidiInOut.cpp",
   "../../plugins/MidiInOut/MidiInOut.h",
   "../../plugins/MidiInOut/MidiInOutEditor.cpp",
   "../../plugins/MidiInOut/MidiInOutEditor.h",
   "../../plugins/common/*.h",
   "../../include/vstsdk2.4/pluginterfaces/vst2.x/aeffect.h",
   "../../include/vstsdk2.4/pluginterfaces/vst2.x/aeffectx.h",
   "../../include/vstsdk2.4/pluginterfaces/vst2.x/vstfxstore.h",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/aeffeditor.h",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/audioeffect.cpp",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/audioeffect.h",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.cpp",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h",
   "../../include/vstsdk2.4/public.sdk/source/vst2.x/vstplugmain.cpp",
  }
  links {
   "portmidi",
   "winmm",
  }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  dofile "../../build/premake/premake-defaults-DLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
  flags { "StaticRuntime" }
