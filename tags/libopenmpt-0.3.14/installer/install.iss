; OpenMPT Install script
; Written by Johannes Schultz
; https://openmpt.org/
; https://sagamusix.de/

; This file cannot be compiled on its own. You need to compile any of these files:
; win32.iss - For generating the standard Win32 setup.
; win64.iss - For generating the standard Win64 setup.

#ifndef PlatformName
#error You must specify which installer to build by compiling either win32.iss or win64.iss
#endif

#define GetAppVersion StringChange(GetFileProductVersion("..\bin\" + PlatformFolder + "\mptrack.exe"), ",", ".")
#define GetAppVersionShort Copy(GetAppVersion, 1, 4)

#ifndef BaseNameAddition
#define BaseNameAddition
#endif

[Setup]
AppVerName=OpenMPT {#GetAppVersionShort} ({#PlatformName})
AppVersion={#GetAppVersion}
AppName=OpenMPT ({#PlatformName})
AppPublisher=OpenMPT Devs
AppPublisherURL=https://openmpt.org/
AppSupportURL=https://forum.openmpt.org/
AppUpdatesURL=https://openmpt.org/
DefaultDirName={pf}\OpenMPT
DisableDirPage=no
DisableProgramGroupPage=yes
OutputDir=.\
OutputBaseFilename=OpenMPT-{#GetAppVersion}-Setup{#BaseNameAddition}
Compression=lzma2
SolidCompression=yes
WizardImageFile=install-big.bmp
WizardSmallImageFile=install-small.bmp
CreateUninstallRegKey=not IsTaskSelected('portable')
Uninstallable=not IsTaskSelected('portable')
UninstallDisplayIcon={app}\mptrack.exe
DisableWelcomePage=yes

[Tasks]
; icons and install mode
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}
Name: startmenuicon; Description: "Create a Start Menu icon"; GroupDescription: {cm:AdditionalIcons}
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: portable; Description: Portable mode (use program folder for storing settings, no registry changes); GroupDescription: Options:; Flags: unchecked
; file associations - put this below all other [tasks]!
#include "filetypes.iss"

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; note: packageTemplate\ contains files specific for the "install package".
; for files that are common with the "zip package", use ..\packageTemplate\

; preserve file type order for best solid compression results (first binary, then text)
; base folder
Source: ..\bin\{#PlatformFolder}\mptrack.exe; DestDir: {app}; Flags: ignoreversion; Check: not InstallWinOld
Source: ..\bin\{#PlatformFolder}\PluginBridge32.exe; DestDir: {app}; Flags: ignoreversion; Check: not InstallWinOld
Source: ..\bin\{#PlatformFolder}\PluginBridge64.exe; DestDir: {app}; Flags: ignoreversion; Check: not InstallWinOld
Source: ..\bin\{#PlatformFolder}\OpenMPT_SoundTouch_f32.dll; DestDir: {app}; Flags: ignoreversion; Check: not InstallWinOld
Source: ..\bin\{#PlatformFolder}\openmpt-mpg123.dll; DestDir: {app}; Flags: ignoreversion; Check: not InstallWinOld
; Additional binaries for XP-/Vista-compatible version
Source: ..\bin\{#PlatformFolderOld}\mptrack.exe; DestDir: {app}; Flags: ignoreversion; Check: InstallWinOld
Source: ..\bin\{#PlatformFolderOld}\PluginBridge32.exe; DestDir: {app}; Flags: ignoreversion; Check: InstallWinOld
Source: ..\bin\{#PlatformFolderOld}\PluginBridge64.exe; DestDir: {app}; Flags: ignoreversion; Check: InstallWinOld
Source: ..\bin\{#PlatformFolderOld}\OpenMPT_SoundTouch_f32.dll; DestDir: {app}; Flags: ignoreversion; Check: InstallWinOld
Source: ..\bin\{#PlatformFolderOld}\openmpt-mpg123.dll; DestDir: {app}; Flags: ignoreversion; Check: InstallWinOld
; Wine support
Source: ..\bin\{#PlatformFolder}\openmpt-wine-support.zip; DestDir: {app}; Flags: ignoreversion

Source: ..\packageTemplate\ExampleSongs\*.*; DestDir: {app}\ExampleSongs\; Flags: ignoreversion sortfilesbyextension recursesubdirs

Source: packageTemplate\readme.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\packageTemplate\History.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\packageTemplate\OpenMPT Manual.chm; DestDir: {app}; Flags: ignoreversion

; release notes
Source: ..\packageTemplate\ReleaseNotesImages\general\*.*; DestDir: {app}\ReleaseNotesImages\general\; Flags: ignoreversion sortfilesbyextension
Source: ..\packageTemplate\ReleaseNotesImages\{#GetAppVersionShort}\*.*; DestDir: {app}\ReleaseNotesImages\{#GetAppVersionShort}\; Flags: ignoreversion sortfilesbyextension
Source: ..\packageTemplate\OMPT_{#GetAppVersionShort}_ReleaseNotes.html; DestDir: {app}; Flags: ignoreversion

; license stuff
Source: ..\packageTemplate\License.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\packageTemplate\Licenses\*.*; DestDir: {app}\Licenses; Flags: ignoreversion sortfilesbyextension

; keymaps
Source: ..\packageTemplate\ExtraKeymaps\*.*; DestDir: {app}\ExtraKeymaps; Flags: ignoreversion sortfilesbyextension

; kind of auto-backup - handy!
Source: {userappdata}\OpenMPT\Keybindings.mkb; DestDir: {userappdata}\OpenMPT; DestName: Keybindings.mkb.old; Flags: external skipifsourcedoesntexist; Tasks: not portable
Source: {userappdata}\OpenMPT\mptrack.ini; DestDir: {userappdata}\OpenMPT; DestName: mptrack.ini.old; Flags: external skipifsourcedoesntexist; Tasks: not portable
Source: {userappdata}\OpenMPT\SongSettings.ini; DestDir: {userappdata}\OpenMPT; DestName: SongSettings.ini.old; Flags: external skipifsourcedoesntexist; Tasks: not portable
Source: {userappdata}\OpenMPT\plugin.cache; DestDir: {userappdata}\OpenMPT; DestName: plugin.cache.old; Flags: external skipifsourcedoesntexist; Tasks: not portable

[Dirs]
; option dirs for non-portable mode
Name: {userappdata}\OpenMPT; Tasks: not portable
Name: {userappdata}\OpenMPT\tunings; Tasks: not portable
; dirst for portable mode
Name: {app}\tunings; Tasks: portable

[Icons]
; start menu
Name: {userprograms}\OpenMPT; Filename: {app}\mptrack.exe; Tasks: startmenuicon

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
Filename: {app}\ModPlug Central.url; Section: InternetShortcut; Key: URL; String: https://forum.openmpt.org/; Flags: createkeyifdoesntexist

[Run]
; duh
Filename: "{app}\OMPT_{#GetAppVersionShort}_ReleaseNotes.html"; Description: "View Release Notes"; Flags: shellexec nowait postinstall skipifsilent
Filename: {app}\mptrack.exe; Parameters: """{code:RandomExampleFile}"""; Description: {cm:LaunchProgram,OpenMPT}; Flags: nowait postinstall skipifsilent

[InstallDelete]
; We no longer require unmo3
Type: files; Name: {app}\unmo3.dll
; i16 -> f32
Type: files; Name: {app}\OpenMPT_SoundTouch_i16.dll
; Old SoundTouch documents
Type: files; Name: {app}\SoundTouch\README.html
Type: files; Name: {app}\SoundTouch\COPYING.TXT
Type: dirifempty; Name: {app}\SoundTouch
; MIDI Input/Output plugin was previously shipped as separate DLL
Type: files; Name: {app}\Plugins\MIDI\MIDI Input Output.dll
Type: dirifempty; Name: {app}\Plugins\MIDI
Type: dirifempty; Name: {app}\Plugins

[UninstallDelete]
; internet shortcut has to be deleted manually
Type: files; Name: {app}\ModPlug Central.url
; normal installation
Type: dirifempty; Name: {userappdata}\OpenMPT\Autosave; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT\TemplateModules; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT\tunings; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT\Components\{#PlatformArchitecture}; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT\Components; Tasks: not portable
Type: dirifempty; Name: {userappdata}\OpenMPT; Tasks: not portable
; portable installation
Type: dirifempty; Name: {app}\Autosave; Tasks: portable
Type: dirifempty; Name: {app}\TemplateModules; Tasks: portable
Type: dirifempty; Name: {app}\tunings; Tasks: portable
Type: dirifempty; Name: {userappdata}\OpenMPT\Components\{#PlatformArchitecture}; Tasks: portable
Type: dirifempty; Name: {userappdata}\OpenMPT\Components; Tasks: portable

#include "utilities.iss"

[Code]
var
    BitnessPage: TInputOptionWizardPage;
    BuildType: Integer;

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

// Picks a random example song file to play
Function RandomExampleFile(Dummy: String): String;
var
    Files: TstringList;
    FindRec: TFindRec;
begin
    Result := '';
    if FindFirst(ExpandConstant('{app}\ExampleSongs\*'), FindRec) then
    try
        Files := TstringList.Create;
        repeat
            if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0 then
            Files.Add(FindRec.Name);
        until not FindNext(FindRec);
        Result := ExpandConstant('{app}\ExampleSongs\') + Files[Random(Files.Count)];
    finally
        FindClose(FindRec);
    end;
end;

Function InstallWinOld(): Boolean;
begin
    Result := (BuildType = 1);
end;

function IsProcessorFeaturePresent(Feature: Integer): Integer;
external 'IsProcessorFeaturePresent@Kernel32.dll stdcall delayload';

function IsWine(): PAnsiChar;
external 'wine_get_version@ntdll.dll stdcall delayload';

procedure InitializeWizard();
var
    IsModernSystem: Boolean;
    WineVersion: PAnsiChar;
begin
    BitnessPage := CreateInputOptionPage(wpWelcome, 'OpenMPT Version', 'Select the version of OpenMPT you want to install.',
        'Select the version of OpenMPT you want to install. Setup already determined the most suitable version for your system.', True, False);

    // Add items
    try
        // Check if installing on Wine 1.8 or later
        WineVersion := IsWine();
        IsModernSystem := ((WineVersion <> nil) and (CompareStr(AnsiString(WineVersion), '1.8') >= 0));
#if PlatformName == "32-Bit"
        BitnessPage.Add('32-Bit, for Wine 1.8 or newer and CPU with SSE2 instruction set');
        BitnessPage.Add('32-Bit, for Wine 1.6 or CPU without SSE2 instruction set');
#else
        BitnessPage.Add('64-Bit, for Wine 1.8 or newer');
        BitnessPage.Add('64-Bit, for Wine 1.6');
#endif
    except
        // Installing on Windows 7 or later
        IsModernSystem := (GetWindowsVersion >= $06010000);
#if PlatformName == "32-Bit"
        BitnessPage.Add('32-Bit, for Windows 7 or newer and CPU with SSE2 instruction set');
        BitnessPage.Add('32-Bit, for Windows XP / Vista or CPU without SSE2 instruction set');
#else
        BitnessPage.Add('64-Bit, for Windows 7 or newer');
        BitnessPage.Add('64-Bit, for Windows XP / Vista');
#endif
    end;
    BitnessPage.Values[1] := True;

    if(IsModernSystem) then
    begin
        try
            if(IsWin64 or (IsProcessorFeaturePresent(10) <> 0)) then
            begin
                // Windows 7 or Wine with SSE2
                BitnessPage.Values[0] := True;
            end;
        except
        end;
    end else

end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
    programfiles: String;
begin
    case CurPageID of
    wpSelectTasks:
        begin
            programfiles := ExpandConstant('{pf}\');
            if((CompareText(programfiles, Copy(ExpandConstant('{app}\'), 0, Length(programfiles))) = 0) and IsTaskSelected('portable')) then
            begin
                MsgBox('Warning: Installing OpenMPT to' #10 + programfiles + #10 'in portable mode may lead to problems if you are not running it with an administrator account!', mbInformation, MB_OK);
            end;
        end;

    BitnessPage.ID:
        begin;
            BuildType := BitnessPage.SelectedValueIndex;
        end;
    end;
    Result := true;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
    case CurStep of
    ssPostInstall:
        begin
            // Copy old config files from app's directory, if possible and necessary.
            CopyConfigsToAppDataDir();
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
            if UninstallSilent() then
            begin
                // Keep user settings if uninstalling silently
                Exit;
            end;
            if MsgBox('Do you want to keep your OpenMPT settings files (mptrack.ini, SongSettings.ini, Keybindings.mkb, plugin.cache and local_tunings.tc)?', mbConfirmation, MB_YESNO or MB_DEFBUTTON1) = IDNO then
            begin
                if(GetIniInt('Paths', 'UseAppDataDirectory', 1, 0, 0, ExpandConstant('{app}\mptrack.ini')) = 1) then
                begin
                    filepath := ExpandConstant('{userappdata}\OpenMPT\');
                end else
                    filepath := ExpandConstant('{app}\');
                begin
                end;
                DeleteFile(filepath + 'mptrack.ini');
                DeleteFile(filepath + 'SongSettings.ini');
                DeleteFile(filepath + 'Keybindings.mkb');
                DeleteFile(filepath + 'plugin.cache');
                DeleteFile(filepath + 'tunings\local_tunings.tc');
                DelTree(filepath + 'Autosave\*.Autosave.*', False, True, False);

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
