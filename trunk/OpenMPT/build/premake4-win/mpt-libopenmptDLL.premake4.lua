 
 project "libopenmptDLL"
  uuid "238ccab7-5880-4172-b0cf-9b9298ed9d29"
  language "C++"
  location ( "../../build/" .. _ACTION )
  targetname "libopenmpt"
  objdir "../../build/obj/libopenmptDLL"
  includedirs {
   "../..",
   "../../common",
   "../../soundlib",
   "../../include",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../common/*.cpp",
   "../../common/*.h",
   "../../soundlib/*.cpp",
   "../../soundlib/*.h",
   "../../libopenmpt/libopenmpt.h",
   "../../libopenmpt/libopenmpt.hpp",
   "../../libopenmpt/libopenmpt_config.h",
   "../../libopenmpt/libopenmpt_ext.hpp",
   "../../libopenmpt/libopenmpt_impl.hpp",
   "../../libopenmpt/libopenmpt_internal.h",
   "../../libopenmpt/libopenmpt_stream_callbacks_fd.h",
   "../../libopenmpt/libopenmpt_stream_callbacks_file.h",
   "../../libopenmpt/libopenmpt_version.h",
   "../../libopenmpt/libopenmpt_c.cpp",
   "../../libopenmpt/libopenmpt_cxx.cpp",
   "../../libopenmpt/libopenmpt_ext.cpp",
   "../../libopenmpt/libopenmpt_impl.cpp",
  }
  flags { "Unicode" }
  defines { "LIBOPENMPT_BUILD", "LIBOPENMPT_BUILD_DLL" }
  links {
   "miniz-shared",
  }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  configuration "vs2008"
   includedirs { "../../include/msinttypes/stdint" }
  dofile "../../build/premake4-win/premake4-defaults-DLL.lua"
  dofile "../../build/premake4-win/premake4-defaults-shared.lua"
