; OpenMPT Install script for InnoSetup
; Written by Johannes Schultz
; http://sagamusix.de/
; http://sagagames.de/

; This file is provided for creating an install package without the proprietary unmo3.dll (for example for the SourceForge package).
; Instead of including the file in the setup package, the user instead has the possibility to automatically download unmo3.dll from
; our servers.
; The download code requires the ISTool IDE with its downloader extension. ISPP is also required.
; To download and install ISPP and ISTool, get the Inno Setup QuickStart Pack from http://www.jrsoftware.org/isdl.php#qsp

#define DOWNLOAD_MO3

#include "install.iss"

[_ISToolDownload]
Source: http://openmpt.com/download/unmo3.dll; DestDir: {tmp}; DestName: openmpt-unmo3.dll.tmp; Tasks: downloadmo3
[Code]
// Verify checksum of downloaded file, and if it is OK, copy it to the app directory.
procedure VerifyUNMO3Checksum();
begin
    if(IsTaskSelected('downloadmo3') And FileExists(ExpandConstant('{tmp}\openmpt-unmo3.dll.tmp'))) then
    begin
	    if(GetSHA1OfFile(ExpandConstant('{tmp}\openmpt-unmo3.dll.tmp')) <> '2e17f7bb6d19ce326851333b918070c5357cacd1') then
    	begin
    	   MsgBox('Warning: unmo3.dll has been downloaded, but its checksum is wrong! This means that either the downloaded file is corrupted or that a newer version of unmo3.dll is available. The file has thus not been installed. Please obtain unmo3.dll from http://openmpt.com/ and verify its checksum.', mbCriticalError, MB_OK)
    	end else
    	begin
    	   FileCopy(ExpandConstant('{tmp}\openmpt-unmo3.dll.tmp'), ExpandConstant('{app}\unmo3.dll'), true);
    	end;
    	DeleteFile(ExpandConstant('{tmp}\openmpt-unmo3.dll.tmp'));
    end;
end;

function IsUNMO3Installed(): Boolean;
begin
	Result := FileExists(ExpandConstant('{app}\unmo3.dll'));
end;

// Function generated by ISTool.
function NextButtonClick(CurPage: Integer): Boolean;
begin
	Result := istool_download(CurPage);
end;
