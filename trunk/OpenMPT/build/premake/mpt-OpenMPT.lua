
if layout == "custom" then
  project "OpenMPT-custom"
  mpt_projectname = "OpenMPT-custom"
   uuid "6b9af880-af37-4268-bb91-2b982ff6499a"
else
if charset == "Unicode" then
if stringmode == "WCHAR" then
  project "OpenMPT"
  mpt_projectname = "OpenMPT"
	uuid "37FC32A4-8DDC-4A9C-A30C-62989DD8ACE9"
else
  project "OpenMPT-UTF8"
  mpt_projectname = "OpenMPT-UTF8"
	uuid "e89507fa-a251-457e-9957-f6b453c77daf"
end
else
	project "OpenMPT-ANSI"
	mpt_projectname = "OpenMPT-ANSI"
	uuid "ba66db50-e2f0-4c9e-b650-0cca6c66e1c1"
end
end
  language "C++"
  location ( "../../build/" .. mpt_projectpathname )
if layout == "custom" then
  vpaths {
		["*"] = "../../",
		["Sources/*"] = "../../**.cpp",
		["Headers/*"] = "../../**.h",
	}
else
  vpaths { ["*"] = "../../" }
end
  dofile "../../build/premake/premake-defaults-EXEGUI.lua"
  dofile "../../build/premake/premake-defaults.lua"
	dofile "../../build/premake/premake-defaults-strict.lua"
if stringmode == "UTF8" then
  filter { "configurations:*Shared" }
   targetname "OpenMPT-UTF8"
  filter { "not configurations:*Shared" }
   targetname "mptrack-UTF8"
elseif charset == "MBCS" then
  filter { "configurations:*Shared" }
   targetname "OpenMPT-ANSI"
  filter { "not configurations:*Shared" }
   targetname "mptrack-ANSI"
else
  filter { "configurations:*Shared" }
   targetname "OpenMPT"
  filter { "not configurations:*Shared" }
   targetname "mptrack"
end
  filter {}
  local extincludedirs = {
   "../../include",
   "../../include/vstsdk2.4",
   "../../include/ASIOSDK2/common",
   "../../include/flac/include",
   "../../include/lame/include",
   "../../include/lhasa/lib/public",
   "../../include/mpg123/ports/MSVC++",
   "../../include/mpg123/src/libmpg123",
   "../../include/nlohmann-json/include",
   "../../include/ogg/include",
   "../../include/opus/include",
   "../../include/opusenc/include",
   "../../include/opusfile/include",
   "../../include/portaudio/include",
   "../../include/rtaudio",
   "../../include/vorbis/include",
   "../../include/zlib",
  }
	filter { "action:vs*" }
		includedirs ( extincludedirs )
	filter { "not action:vs*" }
		sysincludedirs ( extincludedirs )
	filter {}
  includedirs {
   "../../common",
   "../../soundlib",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../mptrack/res/OpenMPT.manifest",
  }
  files {
   "../../common/*.cpp",
   "../../common/*.h",
   "../../soundbase/*.cpp",
   "../../soundbase/*.h",
   "../../soundlib/*.cpp",
   "../../soundlib/*.h",
   "../../soundlib/plugins/*.cpp",
   "../../soundlib/plugins/*.h",
   "../../soundlib/plugins/dmo/*.cpp",
   "../../soundlib/plugins/dmo/*.h",
   "../../sounddsp/*.cpp",
   "../../sounddsp/*.h",
   "../../sounddev/*.cpp",
   "../../sounddev/*.h",
   "../../unarchiver/*.cpp",
   "../../unarchiver/*.h",
   "../../misc/*.cpp",
   "../../misc/*.h",
   "../../mptrack/*.cpp",
   "../../mptrack/*.h",
   "../../mptrack/plugins/*.cpp",
   "../../mptrack/plugins/*.h",
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
	pchheader "stdafx.h"
	pchsource "../../common/stdafx.cpp"
  defines { "MODPLUG_TRACKER" }
  largeaddressaware ( true )
  characterset(charset)
if stringmode == "UTF8" then
	defines { "MPT_USTRING_MODE_UTF8_FORCE" }
end
  flags { "MFC" }
  warnings "Extra"
  links {
   "UnRAR",
   "zlib",
   "minizip",
   "smbPitchShift",
   "lame",
   "lhasa",
   "flac",
   "mpg123",
   "ogg",
   "opus",
   "opusenc",
   "opusfile",
   "portaudio",
   "r8brain",
   "rtaudio",
   "rtmidi",
   "soundtouch",
   "vorbis",
  }
  filter { "configurations:*Shared" }
  filter { "not configurations:*Shared" }
   linkoptions {
    "/DELAYLOAD:openmpt-lame.dll",
    "/DELAYLOAD:openmpt-mpg123.dll",
    "/DELAYLOAD:OpenMPT_SoundTouch_f32.dll",
   }
  filter {}
	if not _OPTIONS["xp"] then
  linkoptions {
   "/DELAYLOAD:mf.dll",
   "/DELAYLOAD:mfplat.dll",
   "/DELAYLOAD:mfreadwrite.dll",
--   "/DELAYLOAD:mfuuid.dll", -- static library
   "/DELAYLOAD:propsys.dll",
  }
	end
  filter { "action:vs*" }
    files {
      "../../build/vs/debug/openmpt.natvis",
    }
  filter {}
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }

 project "OpenMPT-NativeSupport"
  uuid "563a631d-fe07-47bc-a98f-9fe5b3ebabfa"
  language "C++"
  location ( "../../build/" .. mpt_projectpathname )
  vpaths { ["*"] = "../../" }
  mpt_projectname = "OpenMPT-NativeSupport"
  dofile "../../build/premake/premake-defaults-DLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
	dofile "../../build/premake/premake-defaults-strict.lua"
  includedirs {
   "../../common",
   "../../include",
   "../../include/msinttypes/inttypes",
   "../../include/ASIOSDK2/common",
   "../../include/nlohmann-json/include",
   "../../include/portaudio/include",
   "../../include/rtaudio",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../common/*.cpp",
   "../../common/*.h",
   "../../soundbase/*.cpp",
   "../../soundbase/*.h",
   "../../sounddev/*.cpp",
   "../../sounddev/*.h",
   "../../misc/*.cpp",
   "../../misc/*.h",
   "../../mptrack/wine/*.cpp",
   "../../mptrack/wine/*.h",
  }
  excludes {
   "../../mptrack/wine/WineWrapper.cpp",
  }
  defines { "MODPLUG_TRACKER", "MPT_BUILD_WINESUPPORT" }
  largeaddressaware ( true )
  characterset "Unicode"
  warnings "Extra"
  links {
   "portaudio",
   "rtaudio",
  }
  filter {}
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }

 project "OpenMPT-WineWrapper"
  uuid "f3da2bf5-e84a-4f71-80ab-884594863d3a"
  language "C"
  location ( "../../build/" .. mpt_projectpathname )
  vpaths { ["*"] = "../../" }
  mpt_projectname = "OpenMPT-WineWrapper"
  dofile "../../build/premake/premake-defaults-DLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
	dofile "../../build/premake/premake-defaults-strict.lua"
  includedirs {
   "../../common",
   "../../include",
   "../../include/msinttypes/inttypes",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../mptrack/wine/WineWrapper.c",
  }
  defines { "MODPLUG_TRACKER", "MPT_BUILD_WINESUPPORT_WRAPPER" }
  largeaddressaware ( true )
  characterset "Unicode"
  warnings "Extra"
  links {
   "OpenMPT-NativeSupport",
  }
  filter {}
  postbuildcommands { "..\\..\\build\\wine\\build_wine_support.cmd $(IntDir) $(OutDir)" }
