
 project "libopenmpt_modplug"
  uuid "F1397660-35F6-4734-837E-88DCE80E4837"
  language "C++"
  location ( "../../build/" .. _ACTION )
  targetname "libmodplug"
  objdir "../../build/obj/libopenmpt_modplug"
  includedirs {
   "../..",
   "../../include/modplug/include",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../libopenmpt/libopenmpt_modplug.c",
   "../../libopenmpt/libopenmpt_modplug_cpp.cpp",
  }
  links { "libopenmptDLL", "miniz-shared" }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  dofile "../../build/premake/premake-defaults-DLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
  filter {}
   removeflags { "StaticRuntime" }
