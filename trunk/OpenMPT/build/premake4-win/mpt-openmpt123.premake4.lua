 
 project "openmpt123"
  uuid "2879F62E-9E2F-4EAB-AE7D-F60C194DD5CB"
  language "C++"
  location ( "../../build/" .. _ACTION )
  objdir "../../build/obj/openmpt123"
  includedirs {
   "../..",
   "../../openmpt123",
   "../../include/flac/include",
   "../../include/portaudio/include",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../openmpt123/*.cpp",
   "../../openmpt123/*.hpp",
  }
  defines { }
  flags { "Unicode" }
  links {
   "libopenmpt",
   "miniz",
   "flac",
   "portaudio",
   "ksuser",
   "winmm",
  }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  configuration "vs2008"
   includedirs { "../../include/msinttypes/stdint" }
  dofile "../../build/premake4-win/premake4-defaults-EXE.lua"
  dofile "../../build/premake4-win/premake4-defaults-static.lua"
