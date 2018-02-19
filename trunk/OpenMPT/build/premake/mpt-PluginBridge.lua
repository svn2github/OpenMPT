
 project "PluginBridge"
  uuid "1A147336-891E-49AC-9EAD-A750599A224C"
  language "C++"
  location ( "../../build/" .. mpt_projectpathname )
  vpaths { ["*"] = "../../pluginBridge/" }
  mpt_projectname = "PluginBridge"
  dofile "../../build/premake/premake-defaults-EXEGUI.lua"
  dofile "../../build/premake/premake-defaults.lua"
	dofile "../../build/premake/premake-defaults-strict.lua"
  local extincludedirs = {
   "../include/vstsdk2.4",
  }
	filter { "action:vs*" }
		includedirs ( extincludedirs )
	filter { "not action:vs*" }
		sysincludedirs ( extincludedirs )
	filter {}
  includedirs {
   "../../common",
   "$(IntDir)/svn_version",
   "../../build/svn_version",
  }
  files {
   "../../pluginBridge/AEffectWrapper.h",
   "../../pluginBridge/Bridge.cpp",
   "../../pluginBridge/Bridge.h",
   "../../pluginBridge/BridgeCommon.h",
   "../../pluginBridge/BridgeOpCodes.h",
   "../../common/versionNumber.h",
  }
  files {
   "../../pluginBridge/PluginBridge.rc",
  }
  files {
   "../../pluginBridge/PluginBridge.manifest",
  }
  defines { "MODPLUG_TRACKER" }
  largeaddressaware ( true )
  characterset "Unicode"
  warnings "Extra"
  prebuildcommands { "..\\..\\build\\svn_version\\update_svn_version_vs_premake.cmd $(IntDir)" }
  filter { "architecture:x86" }
   targetsuffix "32"
  filter { "architecture:x86_64" }
   targetsuffix "64"
  filter {}
	filter { "action:vs*", "architecture:x86_64" }
		linkoptions { "/HIGHENTROPYVA:NO" }
	filter {}

	if _OPTIONS["win10"] then
		filter { "architecture:x86" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-64-win10\" mkdir \"$(TargetDir)\\..\\x86-64-win10\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-64-win10\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-64-win10\\$(TargetName).pdb\"",
			}
		filter { "architecture:x86_64" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-32-win10\" mkdir \"$(TargetDir)\\..\\x86-32-win10\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-32-win10\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-32-win10\\$(TargetName).pdb\"",
			}
	elseif _OPTIONS["xp"] then
		filter { "architecture:x86" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-64-winxp64\" mkdir \"$(TargetDir)\\..\\x86-64-winxp64\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-64-winxp64\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-64-winxp64\\$(TargetName).pdb\"",
			}
		filter { "architecture:x86_64" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-32-winxp\" mkdir \"$(TargetDir)\\..\\x86-32-winxp\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-32-winxp\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-32-winxp\\$(TargetName).pdb\"",
			}
	else
		filter { "architecture:x86" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-64-win7\" mkdir \"$(TargetDir)\\..\\x86-64-win7\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-64-win7\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-64-win7\\$(TargetName).pdb\"",
			}
		filter { "architecture:x86_64" }
			postbuildcommands {
				"if not exist \"$(TargetDir)\\..\\x86-32-win7\" mkdir \"$(TargetDir)\\..\\x86-32-win7\"",
				"copy /y \"$(TargetDir)\\$(TargetFileName)\" \"$(TargetDir)\\..\\x86-32-win7\\$(TargetFileName)\"",
				"copy /y \"$(TargetDir)\\$(TargetName).pdb\" \"$(TargetDir)\\..\\x86-32-win7\\$(TargetName).pdb\"",
			}
	end

	filter {}

