
if _ACTION ~= "vs2008" then

 project "in_openmpt"
  uuid "D75AEB78-5537-49BD-9085-F92DEEFA84E8"
  language "C++"
  location ( "../../build/" .. _ACTION )
  objdir "../../build/obj/in_openmpt"
  includedirs {
   "../..",
   "../../include",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../libopenmpt/in_openmpt.cpp",
   "../../libopenmpt/libopenmpt_settings.hpp",
   "../../libopenmpt/libopenmpt_settings.cpp",
   "../../libopenmpt/libopenmpt_settings.rc",
   "../../libopenmpt/resource.h",
  }
  flags { "MFC", "Unicode" }
  links { "libopenmpt", "miniz" }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  dofile "../../build/premake/premake-defaults-DLL.lua"
  dofile "../../build/premake/premake-defaults.lua"

end
