
 project "pugixml"
  uuid "07B89124-7C71-42cc-81AB-62B09BB61F9B"
  language "C++"
  location ( "../../build/" .. _ACTION .. "-ext" )
  objdir "../../build/obj/pugixml"
  includedirs { }
  files {
   "../../include/pugixml/src/pugixml.cpp",
  }
  files {
   "../../include/pugixml/src/pugiconfig.hpp",
   "../../include/pugixml/src/pugixml.hpp",
  }
  dofile "../../build/premake4-win/premake4-defaults-LIB.lua"
  dofile "../../build/premake4-win/premake4-defaults-static.lua"
