 
 project "libopenmpt-full"
  uuid "76c962ad-c957-46c2-9b66-b74e476ecabc"
  language "C++"
  location ( "../../build/" .. mpt_projectpathname )
  vpaths { ["*"] = "../../" }
  mpt_projectname = "libopenmpt-full"
  dofile "../../build/premake/premake-defaults-LIBorDLL.lua"
  dofile "../../build/premake/premake-defaults.lua"
  local extincludedirs = {
   "../../include",
   "../../include/mpg123/ports/MSVC++",
   "../../include/mpg123/src/libmpg123",
   "../../include/ogg/include",
   "../../include/vorbis/include",
   "../../include/zlib",
  }
  filter { "action:vs*" }
    includedirs ( extincludedirs )
  filter { "not action:vs*" }
    sysincludedirs ( extincludedirs )
  filter {}
  includedirs {
   "../..",
   "../../common",
   "../../soundlib",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
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
   "../../libopenmpt/libopenmpt.h",
   "../../libopenmpt/libopenmpt.hpp",
   "../../libopenmpt/libopenmpt_config.h",
   "../../libopenmpt/libopenmpt_ext.h",
   "../../libopenmpt/libopenmpt_ext.hpp",
   "../../libopenmpt/libopenmpt_ext_impl.hpp",
   "../../libopenmpt/libopenmpt_impl.hpp",
   "../../libopenmpt/libopenmpt_internal.h",
   "../../libopenmpt/libopenmpt_stream_callbacks_buffer.h",
   "../../libopenmpt/libopenmpt_stream_callbacks_fd.h",
   "../../libopenmpt/libopenmpt_stream_callbacks_file.h",
   "../../libopenmpt/libopenmpt_version.h",
   "../../libopenmpt/libopenmpt_c.cpp",
   "../../libopenmpt/libopenmpt_cxx.cpp",
   "../../libopenmpt/libopenmpt_ext_impl.cpp",
   "../../libopenmpt/libopenmpt_impl.cpp",
  }

	filter { "action:vs*" }
		resdefines {
			"MPT_BUILD_VER_SPECIAL_PREFIX=\"+full\"",
		}
	filter { "action:vs*", "kind:SharedLib or ConsoleApp or WindowedApp" }
		resdefines {
			"MPT_BUILD_VER_FILENAME=\"" .. mpt_projectname .. ".dll\"",
			"MPT_BUILD_VER_FILEDESC=\"" .. mpt_projectname .. "\"",
		}
	filter { "action:vs*", "kind:SharedLib or ConsoleApp or WindowedApp" }
		resincludedirs {
			"$(IntDir)/svn_version",
			"../../build/svn_version",
			"$(ProjDir)/../../build/svn_version",
		}
		files {
			"../../libopenmpt/libopenmpt_version.rc",
		}
	filter { "action:vs*", "kind:SharedLib" }
		resdefines { "MPT_BUILD_VER_DLL" }
	filter { "action:vs*", "kind:ConsoleApp or WindowedApp" }
		resdefines { "MPT_BUILD_VER_EXE" }
	filter {}

  characterset "Unicode"
  flags { "ExtraWarnings" }
  defines { "LIBOPENMPT_BUILD", "LIBOPENMPT_BUILD_FULL" }
	defines { "MPT_ENABLE_DLOPEN" }
  filter { "kind:SharedLib" }
   defines { "LIBOPENMPT_BUILD_DLL" }
  filter { "kind:SharedLib" }
		if not _OPTIONS["xp"] then
			links { "delayimp" }
			linkoptions {
				"/DELAYLOAD:mf.dll",
				"/DELAYLOAD:mfplat.dll",
				"/DELAYLOAD:mfreadwrite.dll",
				--"/DELAYLOAD:mfuuid.dll", -- static library
				"/DELAYLOAD:propsys.dll",
			}
		end
  filter {}
  links {
   "vorbis",
   "ogg",
   "mpg123",
   "zlib",
  }
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
