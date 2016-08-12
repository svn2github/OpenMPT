
 project "libopenmpt_modplug"
  uuid "F1397660-35F6-4734-837E-88DCE80E4837"
  language "C++"
  location ( "../../build/" .. mpt_projectpathname )
  vpaths { ["*"] = "../../libopenmpt/" }
  objdir "../../build/obj/libopenmpt_modplug"
  dofile "../../build/premake/premake-defaults-LIBorDLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
  targetname "libmodplug"
  local extincludedirs = {
   "../..",
   "../../include/modplug/include",
  }
	filter { "action:vs*" }
		includedirs ( extincludedirs )
	filter { "not action:vs*" }
		sysincludedirs ( extincludedirs )
	filter {}
  includedirs {
   "../..",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../libopenmpt/libopenmpt_modplug.c",
   "../../libopenmpt/libopenmpt_modplug_cpp.cpp",
  }
	filter {}
	filter { "action:vs*", "not action:vs2008" }
		characterset "Unicode"
		flags { "Unicode" }
	filter { "action:vs*", "action:vs2008" }
		characterset "MBCS"
	filter {}
  links {
   "libopenmpt"
  }
  filter { "configurations:*Shared" }
   defines { "LIBOPENMPT_USE_DLL" }
  filter {}
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
