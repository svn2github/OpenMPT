; OpenMPT Install script
; Written by Johannes Schultz
; http://openmpt.org/
; http://sagamusix.de/

; ISPP is needed for automated version retrieval. Since InnoSetup 5.4.1, ISPP is included in the default InnoSetup installer.
; Furthermore, either the ISTool IDE or InnoIDE with their downloader extensions are required for "unmo3-free" packages which don't contain unmo3.dll, but download it from a server.
; Check install-unmo3-free.iss (ISTool) and install-unmo3-free-itd.iss (InnoIDE) for details on this matter.
; To download and install InnoIDE, get the Inno Setup QuickStart Pack from http://www.jrsoftware.org/isdl.php#qsp

#define GetAppVersion StringChange(GetFileProductVersion("..\bin\Win32\mptrack.exe"), ",", ".")
#define GetAppVersionShort Copy(GetAppVersion, 1, 4)

#ifndef BaseNameAddition
#define BaseNameAddition
#endif

[Setup]
AppId={{67903736-E9BB-4664-B148-F62BCAB4FA42}
AppVerName=OpenMPT {#GetAppVersionShort}
AppVersion={#GetAppVersion}
AppName=OpenMPT
AppPublisher=OpenMPT Devs / Olivier Lapicque
AppPublisherURL=http://openmpt.org/
AppSupportURL=http://forum.openmpt.org/
AppUpdatesURL=http://openmpt.org/
DefaultDirName={pf}\OpenMPT
DefaultGroupName=OpenMPT
AllowNoIcons=yes
OutputDir=.\
OutputBaseFilename=OpenMPT-{#GetAppVersion}-Setup{#BaseNameAddition}
Compression=lzma2
SolidCompression=yes
WizardImageFile=install-big.bmp
WizardSmallImageFile=install-small.bmp
CreateUninstallRegKey=not IsTaskSelected('portable')
;LicenseFile=license.txt
;The following setting is recommended by the Aero wizard guidelines.
DisableWelcomePage=yes

[Tasks]
; icons and install mode
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
#ifdef DOWNLOAD_MO3
Name: downloadmo3; Description: Download unmo3 (library needed for reading MO3 files, recommended); GroupDescription: Options:
#endif
Name: update_c; Description: Automatically check for updates; GroupDescription: Options:
Name: portable; Description: Portable mode (use program folder for storing settings, no registry changes); GroupDescription: Options:; Flags: unchecked
Name: vst_scan; Description: Scan for previously installed VST plugins; GroupDescription: Options:; Flags: unchecked
; file associations - put this below all other [tasks]!
#include "filetypes.iss"

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; note: packageTemplate\ contains files specific for the "install package".
; for files that are common with the "zip package", use ..\packageTemplate\

; preserve file type order for best solid compression results (first binary, then text)
; home folder
Source: ..\bin\Win32\mptrack.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\Win32\OpenMPT_SoundTouch_f32.dll; DestDir: {app}; Flags: ignoreversion
#ifndef DOWNLOAD_MO3
Source: ..\bin\Win32\unmo3.dll; DestDir: {app}; Flags: ignoreversion
#endif

; Plugins
Source: ..\packageTemplate\Plugins\*.*; DestDir: {app}\Plugins\; Flags: ignoreversion recursesubdirs createallsubdirs

Source: ..\packageTemplate\ExampleSongs\*.*; DestDir: {app}\ExampleSongs\; Flags: ignoreversion sortfilesbyextension

Source: packageTemplate\readme.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\packageTemplate\History.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\packageTemplate\OpenMPT Manual.pdf; DestDir: {app}; Flags: ignoreversion

; release notes
Source: ..\packageTemplate\ReleaseNotesImages\general\*.*; DestDir: {app}\ReleaseNotesImages\general\; Flags: ignoreversion sortfilesbyextension
Source: ..\packageTemplate\ReleaseNotesImages\{#GetAppVersionShort}\*.*; DestDir: {app}\ReleaseNotesImages\{#GetAppVersionShort}\; Flags: ignoreversion sortfilesbyextension
Source: ..\packageTemplate\OMPT_{#GetAppVersionShort}_ReleaseNotes.html; DestDir: {app}; Flags: ignoreversion

; soundtouch license stuff
Source: ..\packageTemplate\SoundTouch\*.*; DestDir: {app}\SoundTouch; Flags: ignoreversion sortfilesbyextension
; keymaps
Source: ..\packageTemplate\ExtraKeymaps\*.*; DestDir: {app}\ExtraKeymaps; Flags: ignoreversion sortfilesbyextension

; kind of auto-backup - handy!
Source: {userappdata}\OpenMPT\Keybindings.mkb; DestDir: {userappdata}\OpenMPT; DestName: Keybindings.mkb.old; Flags: external skipifsourcedoesntexist; Tasks: not portable
Source: {userappdata}\OpenMPT\mptrack.ini; DestDir: {userappdata}\OpenMPT; DestName: mptrack.ini.old; Flags: external skipifsourcedoesntexist; Tasks: not portable
Source: {userappdata}\OpenMPT\plugin.cache; DestDir: {userappdata}\OpenMPT; DestName: plugin.cache.old; Flags: external skipifsourcedoesntexist; Tasks: not portable

[Dirs]
; option dirs for non-portable mode
Name: {userappdata}\OpenMPT; Tasks: not portable
Name: {userappdata}\OpenMPT\tunings; Tasks: not portable
; dirst for portable mode
Name: {app}\tunings; Tasks: portable

[Icons]
; start menu
Name: {group}\{cm:LaunchProgram,OpenMPT}; Filename: {app}\mptrack.exe
Name: {group}\{cm:UninstallProgram,OpenMPT}; Filename: {uninstallexe}
Name: {group}\ModPlug Central; Filename: {app}\ModPlug Central.url

; app's directory and keymaps directory (for ease of use)
Name: {app}\Configuration files; Filename: {userappdata}\OpenMPT\; Tasks: not portable
Name: {userappdata}\OpenMPT\More Keymaps; Filename: {app}\extraKeymaps\; Tasks: not portable

; desktop, quick launch
Name: {userdesktop}\OpenMPT; Filename: {app}\mptrack.exe; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\OpenMPT; Filename: {app}\mptrack.exe; Tasks: quicklaunchicon

[INI]
; enable portable mode
Filename: {app}\mptrack.ini; Section: Paths; Key: UseAppDataDirectory; String: 0; Flags: createkeyifdoesntexist; Tasks: portable
; internet shortcut
Filename: {app}\ModPlug Central.url; Section: InternetShortcut; Key: URL; String: http://forum.openmpt.org/; Flags: createkeyifdoesntexist

[Run]
; duh
Filename: "https://sourceforge.net/projects/kernelex/"; Description: "Download KernelEx (required on Windows 98 / Me)"; Flags: shellexec nowait postinstall skipifsilent; Check: not UsingWinNT();
Filename: "{app}\OMPT_{#GetAppVersionShort}_ReleaseNotes.html"; Description: "View Release Notes"; Flags: shellexec nowait postinstall skipifsilent
Filename: {app}\mptrack.exe; Parameters: """{app}\ExampleSongs\manwe - evening glow.it"""; Description: {cm:LaunchProgram,OpenMPT}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; internet shortcut has to be deleted manually
Type: files; Name: {app}\ModPlug Central.url
; normal installation
Type: dirifempty; Name: {userappdata}\OpenMPT\TemplateModules; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT\tunings; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT; Tasks: not portable
; portable installation
Type: dirifempty; Name: {app}\TemplateModules; Tasks: portable
Type: dirifempty; Name: {app}\tunings; Tasks: portable
#ifdef DOWNLOAD_MO3
Type: files; Name: {app}\unmo3.dll; Tasks: downloadmo3
#endif

#include "utilities.iss"
#include "vst_scan.iss"
#include "plugins.iss"

[Code]

#ifdef DOWNLOAD_MO3
procedure VerifyUNMO3Checksum(); forward;
#endif

// Copy old config files to the AppData directory, if there are any (and if the files don't exist already)
procedure CopyConfigsToAppDataDir();
var
    adjustIniPath: Boolean;
    keyFile: String;

begin

    // Not needed if portable mode is enabled.
    if(IsTaskSelected('portable')) then
    begin
        Exit;
    end;

    // If there was an INI file with portable mode flag set, we have to reset it (or else, the mptrack.ini in %appdata% will never be used!)
    if(IniKeyExists('Paths', 'UseAppDataDirectory', ExpandConstant('{app}\mptrack.ini'))) then
    begin
        DeleteIniEntry('Paths', 'UseAppDataDirectory', ExpandConstant('{app}\mptrack.ini'));
    end;

    FileCopy(ExpandConstant('{app}\mptrack.ini'), ExpandConstant('{userappdata}\OpenMPT\mptrack.ini'), true);
    FileCopy(ExpandConstant('{app}\plugin.cache'), ExpandConstant('{userappdata}\OpenMPT\plugin.cache'), true);
    FileCopy(ExpandConstant('{app}\mpt_intl.ini'), ExpandConstant('{userappdata}\OpenMPT\mpt_intl.ini'), true);
    adjustIniPath := FileCopy(ExpandConstant('{app}\Keybindings.mkb'), ExpandConstant('{userappdata}\OpenMPT\Keybindings.mkb'), true);
    adjustIniPath := adjustIniPath or FileCopy(ExpandConstant('{app}\default.mkb'), ExpandConstant('{userappdata}\OpenMPT\Keybindings.mkb'), true);

    // If the keymappings moved, we might have to update the path in the INI file.
    keyFile := GetIniString('Paths', 'Key_Config_File', '', ExpandConstant('{userappdata}\OpenMPT\mptrack.ini'));
    if(((keyFile = ExpandConstant('{app}\Keybindings.mkb')) or (keyFile = ExpandConstant('{app}\default.mkb'))) and (adjustIniPath)) then
    begin
        SetIniString('Paths', 'Key_Config_File', ExpandConstant('{userappdata}\OpenMPT\Keybindings.mkb'), ExpandConstant('{userappdata}\OpenMPT\mptrack.ini'));
    end;

end;

procedure CurStepChanged(CurStep: TSetupStep);
var
    INIFile: String;
    keyboardFilepath: String;
    baseLanguage: Integer;

begin
    // Get the right INI path.
    INIFile := GetINIPath();

    case CurStep of
    ssPostInstall:
        begin

#ifdef DOWNLOAD_MO3
            VerifyUNMO3Checksum();
#endif

            RegisterPlugin('MIDI\MIDI Input Output.dll');

            // Copy old config files from app's directory, if possible and necessary.
            CopyConfigsToAppDataDir();

            // Find a suitable keyboard layout (might not be very precise sometimes, as it's based on the UI language)
            // Check http://msdn.microsoft.com/en-us/library/ms776294%28VS.85%29.aspx for the correct language codes.
            keyboardFilepath := '';
            baseLanguage := (GetUILanguage and $3FF);
            case baseLanguage of
            $07:  // German
                begin
                    keyboardFilepath := 'DE_jojo';
                end;
            $0a:  // Spanish
                begin
                    // Spanish latin-american keymap, so we ignore Spain.
                    if(GetUILanguage <> $0c0a) then
                    begin
                        keyboardFilepath := 'es-LA_mpt_(jmkz)';
                    end;
                end;
            $0c:  // French
                begin
                    keyboardFilepath := 'FR_mpt_(legovitch)';
                end;
            $14:  // Norwegian
                begin
                    keyboardFilepath := 'NO_mpt_classic_(rakib)';
                end;
            end;

            // Found an alternative keybinding.
            if(keyboardFilepath <> '') then
            begin
                FileCopy(ExpandConstant('{app}\extraKeymaps\') + keyboardFilepath + '.mkb', ExtractFilePath(INIFile) + 'Keybindings.mkb', true);
            end;

            // Update check
            if(not IsTaskSelected('update_c')) then
            begin
                SetIniString('Update', 'UpdateCheckPeriod', '0', INIFile);
            end else
            begin
                SetIniString('Update', 'UpdateCheckPeriod', '7', INIFile);
                //SetIniString('Update', 'LastUpdateCheck', GetDateTimeString('yyyy-mm-dd hh:nn', #0, #0), INIFile);
            end;

            // Scan for pre-installed VST plugins
            if(IsTaskSelected('vst_scan')) then
            begin
                OnVSTScan(INIFile);
            end;
        end;
    end;
end;

// Crappy workaround for uninstall stuff
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
    filepath: String;

begin
    case CurUninstallStep of
    usUninstall:
        begin
            if MsgBox('Do you want to keep your OpenMPT settings files (mptrack.ini, Keybindings.mkb, plugin.cache and local_tunings.tc)?', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDNO then
            begin
                if(GetIniInt('Paths', 'UseAppDataDirectory', 1, 0, 0, ExpandConstant('{app}\mptrack.ini')) = 1) then
                begin
                    filepath := ExpandConstant('{userappdata}\OpenMPT\');
                end else
                    filepath := ExpandConstant('{app}\');
                begin
                end;
                DeleteFile(filepath + 'mptrack.ini');
                DeleteFile(filepath + 'Keybindings.mkb');
                DeleteFile(filepath + 'plugin.cache');
                DeleteFile(filepath + 'tunings\local_tunings.tc');

                filepath := GetTempDir();
                if(filepath <> '') then
                begin
                  filepath := filepath + 'OpenMPT Crash Files\';
                  DelTree(filepath, True, True, True);
                end;
            end;
        end;
    end;
end;
