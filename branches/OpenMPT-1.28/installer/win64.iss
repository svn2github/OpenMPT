#define PlatformFolder    "release\vs2015-static\x86-64-win7"
#define PlatformFolderOld "release\vs2015-static\x86-64-winxp64"
#define PlatformName "64-Bit"
#define BaseNameAddition "-x64"
#define PlatformArchitecture "x64"

[Setup]
AppId={{9814C59D-8CBE-4C38-8A5F-7BF9B4FFDA6D}
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

#include "install.iss"
