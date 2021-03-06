// Purely user interface function. Gets and returns user input.
UIASKREP_RESULT uiAskReplace(wchar *Name,size_t MaxNameSize,int64 FileSize,RarTime *FileTime,uint Flags)
{
  bool AllowRename=(Flags & UIASKREP_F_NORENAME)==0;
  eprintf(St(MFileExists),Name);
  int Choice=0;
  do
  {
    Choice=Ask(St(AllowRename ? MYesNoAllRenQ : MYesNoAllQ));
  } while (Choice==0); // 0 means invalid input.
  switch(Choice)
  {
    case 1:
      return UIASKREP_R_REPLACE;
    case 2:
      return UIASKREP_R_SKIP;
    case 3:
      return UIASKREP_R_REPLACEALL;
    case 4:
      return UIASKREP_R_SKIPALL;
  }
  if (AllowRename && Choice==5)
  {
    mprintf(St(MAskNewName));
    if (getwstr(Name,MaxNameSize))
      return UIASKREP_R_RENAME;
    else
      return UIASKREP_R_SKIP; // Process fwgets failure as if user answered 'No'.
  }
  return UIASKREP_R_CANCEL;
}




void uiStartArchiveExtract(bool Extract,const wchar *ArcName)
{
  mprintf(St(Extract ? MExtracting : MExtrTest), ArcName);
}


bool uiStartFileExtract(const wchar *FileName,bool Extract,bool Test,bool Skip)
{
  return true;
}


void uiExtractProgress(int64 CurFileSize,int64 TotalFileSize,int64 CurSize,int64 TotalSize)
{
  int CurPercent=ToPercent(CurSize,TotalSize);
  mprintf(L"\b\b\b\b%3d%%",CurPercent);
}


void uiProcessProgress(const char *Command,int64 CurSize,int64 TotalSize)
{
  int CurPercent=ToPercent(CurSize,TotalSize);
  mprintf(L"\b\b\b\b%3d%%",CurPercent);
}


void uiMsgStore::Msg()
{
  switch(Code)
  {
    case UIERROR_SYSERRMSG:
    case UIERROR_GENERALERRMSG:
      Log(NULL,L"\n%ls",Str[0]);
      break;
    case UIERROR_CHECKSUM:
      Log(Str[0],St(MCRCFailed),Str[1]);
      break;
    case UIERROR_CHECKSUMENC:
      Log(Str[0],St(MEncrBadCRC),Str[1]);
      break;
    case UIERROR_CHECKSUMPACKED:
      Log(Str[0],St(MDataBadCRC),Str[1],Str[0]);
      break;
    case UIERROR_BADPSW:
      Log(Str[0],St(MWrongPassword));
      break;
    case UIERROR_MEMORY:
      Log(NULL,St(MErrOutMem));
      break;
    case UIERROR_FILEOPEN:
      Log(Str[0],St(MCannotOpen),Str[1]);
      break;
    case UIERROR_FILECREATE:
      Log(Str[0],St(MCannotCreate),Str[1]);
      break;
    case UIERROR_FILECLOSE:
      Log(NULL,St(MErrFClose),Str[0]);
      break;
    case UIERROR_FILESEEK:
      Log(NULL,St(MErrSeek),Str[0]);
      break;
    case UIERROR_FILEREAD:
      Log(Str[0],St(MErrRead),Str[1]);
      break;
    case UIERROR_FILEWRITE:
      Log(Str[0],St(MErrWrite),Str[1]);
      break;
#ifndef SFX_MODULE
    case UIERROR_FILEDELETE:
      Log(Str[0],St(MCannotDelete),Str[1]);
      break;
    case UIERROR_FILERENAME:
      Log(Str[0],St(MErrRename),Str[1],Str[2]);
      break;
#endif
    case UIERROR_FILEATTR:
      Log(Str[0],St(MErrChangeAttr),Str[1]);
      break;
    case UIERROR_FILECOPY:
      Log(Str[0],St(MCopyError),Str[1],Str[2]);
      break;
    case UIERROR_FILECOPYHINT:
      Log(Str[0],St(MCopyErrorHint));
      break;
    case UIERROR_DIRCREATE:
      Log(Str[0],St(MExtrErrMkDir),Str[1]);
      break;
    case UIERROR_SLINKCREATE:
      Log(Str[0],St(MErrCreateLnkS),Str[1]);
      break;
    case UIERROR_HLINKCREATE:
      Log(NULL,St(MErrCreateLnkH),Str[0]);
      break;
    case UIERROR_NEEDADMIN:
      Log(NULL,St(MNeedAdmin));
      break;
    case UIERROR_ARCBROKEN:
      Log(Str[0],St(MErrBrokenArc));
      break;
    case UIERROR_HEADERBROKEN:
      Log(Str[0],St(MHeaderBroken));
      break;
    case UIERROR_MHEADERBROKEN:
      Log(Str[0],St(MMainHeaderBroken));
      break;
    case UIERROR_FHEADERBROKEN:
      Log(Str[0],St(MLogFileHead),Str[1]);
      break;
    case UIERROR_SUBHEADERBROKEN:
      Log(Str[0],St(MSubHeadCorrupt));
      break;
    case UIERROR_SUBHEADERUNKNOWN:
      Log(Str[0],St(MSubHeadUnknown));
      break;
    case UIERROR_SUBHEADERDATABROKEN:
      Log(Str[0],St(MSubHeadDataCRC),Str[1]);
      break;
    case UIERROR_RRDAMAGED:
      Log(Str[0],St(MRRDamaged));
      break;
    case UIERROR_UNKNOWNMETHOD:
      Log(Str[0],St(MUnknownMeth),Str[1]);
      break;
    case UIERROR_UNKNOWNENCMETHOD:
      Log(Str[0],St(MUnkEncMethod),Str[1]);
      break;
#ifndef SFX_MODULE
   case UIERROR_RENAMING:
      Log(Str[0],St(MRenaming),Str[1],Str[2]);
      break;
    case UIERROR_NEWERRAR:
      Log(Str[0],St(MNewerRAR));
      break;
#endif
    case UIERROR_RECVOLDIFFSETS:
      Log(NULL,St(MRecVolDiffSets),Str[0],Str[1]);
      break;
    case UIERROR_RECVOLALLEXIST:
      mprintf(St(MRecVolAllExist));
      break;
    case UIERROR_RECONSTRUCTING:
      mprintf(St(MReconstructing));
      break;
    case UIERROR_RECVOLCANNOTFIX:
      mprintf(St(MRecVolCannotFix));
      break;
    case UIERROR_UNEXPEOF:
      Log(Str[0],St(MLogUnexpEOF));
      break;
    case UIERROR_BADARCHIVE:
      Log(Str[0],St(MBadArc),Str[0]);
      break;
    case UIERROR_CMTBROKEN:
      Log(Str[0],St(MLogCommBrk));
      break;
    case UIERROR_INVALIDNAME:
      Log(Str[0],St(MInvalidName),Str[1]);
      break;
#ifndef SFX_MODULE
    case UIERROR_NEWRARFORMAT:
      Log(Str[0],St(MNewRarFormat));
      break;
#endif
    case UIERROR_NOFILESTOEXTRACT:
      mprintf(St(MExtrNoFiles));
      break;
    case UIERROR_MISSINGVOL:
      Log(Str[0],St(MAbsNextVol),Str[0]);
      break;
#ifndef SFX_MODULE
    case UIERROR_NEEDPREVVOL:
      Log(Str[0],St(MUnpCannotMerge),Str[1]);
      break;
    case UIERROR_UNKNOWNEXTRA:
      Log(Str[0],St(MUnknownExtra),Str[1]);
      break;
#endif
#if !defined(SFX_MODULE) && defined(_WIN_ALL)
    case UIERROR_NTFSREQUIRED:
      Log(NULL,St(MNTFSRequired),Str[0]);
      break;
#endif
#if !defined(SFX_MODULE) && defined(_WIN_ALL)
    case UIERROR_ACLBROKEN:
      Log(Str[0],St(MACLBroken),Str[1]);
      break;
    case UIERROR_ACLUNKNOWN:
      Log(Str[0],St(MACLUnknown),Str[1]);
      break;
    case UIERROR_ACLSET:
      Log(Str[0],St(MACLSetError),Str[1]);
      break;
    case UIERROR_STREAMBROKEN:
      Log(Str[0],St(MStreamBroken),Str[1]);
      break;
    case UIERROR_STREAMUNKNOWN:
      Log(Str[0],St(MStreamUnknown),Str[1]);
      break;
#endif
    case UIERROR_INCOMPATSWITCH:
      mprintf(St(MIncompatSwitch),Str[0],Num[0]);
      break;
    case UIERROR_PATHTOOLONG:
      Log(NULL,L"\n%ls%ls%ls",Str[0],Str[1],Str[2]);
      Log(NULL,St(MPathTooLong));
      break;
#ifndef SFX_MODULE
    case UIERROR_DIRSCAN:
      Log(NULL,St(MScanError),Str[0]);
      break;
#endif
    case UIERROR_UOWNERBROKEN:
      Log(Str[0],St(MOwnersBroken),Str[1]);
      break;
    case UIERROR_UOWNERGETOWNERID:
      Log(Str[0],St(MErrGetOwnerID),Str[1]);
      break;
    case UIERROR_UOWNERGETGROUPID:
      Log(Str[0],St(MErrGetGroupID),Str[1]);
      break;
    case UIERROR_UOWNERSET:
      Log(Str[0],St(MSetOwnersError),Str[1]);
      break;
    case UIERROR_ULINKREAD:
      Log(NULL,St(MErrLnkRead),Str[0]);
      break;
    case UIERROR_ULINKEXIST:
      Log(NULL,St(MSymLinkExists),Str[0]);
      break;


#ifndef SFX_MODULE
    case UIMSG_STRING:
      mprintf(L"\n%s",Str[0]);
      break;
#endif
    case UIMSG_CORRECTINGNAME:
      Log(Str[0],St(MCorrectingName));
      break;
    case UIMSG_BADARCHIVE:
      mprintf(St(MBadArc),Str[0]);
      break;
    case UIMSG_CREATING:
      mprintf(St(MCreating),Str[0]);
      break;
    case UIMSG_RENAMING:
      mprintf(St(MRenaming),Str[0],Str[1]);
      break;
    case UIMSG_RECVOLCALCCHECKSUM:
      mprintf(St(MCalcCRCAllVol));
      break;
    case UIMSG_RECVOLFOUND:
      mprintf(St(MRecVolFound),Num[0]);
      break;
    case UIMSG_RECVOLMISSING:
      mprintf(St(MRecVolMissing),Num[0]);
      break;
    case UIMSG_MISSINGVOL:
      mprintf(St(MAbsNextVol),Str[0]);
      break;
    case UIMSG_RECONSTRUCTING:
      mprintf(St(MReconstructing));
      break;
    case UIMSG_CHECKSUM:
      mprintf(St(MCRCFailed),Str[0]);
      break;




    case UIEVENT_RRTESTING:
      mprintf(L"%s      ",St(MTestingRR));
      break;
  }
}


bool uiGetPassword(UIPASSWORD_TYPE Type,const wchar *FileName,SecPassword *Password)
{
  return GetConsolePassword(Type,FileName,Password);
}


void uiAlarm(UIALARM_TYPE Type)
{
  if (uiSoundEnabled)
  {
    static clock_t LastTime=clock();
    if ((clock()-LastTime)/CLOCKS_PER_SEC>5)
    {
#ifdef _WIN_ALL
      MessageBeep(-1);
#else
      putwchar('\007');
#endif
    }
  }
}




bool uiAskNextVolume(wchar *VolName,size_t MaxSize)
{
  eprintf(St(MAskNextVol),VolName);
  return Ask(St(MContinueQuit))!=2;
}


bool uiAskRepeatRead(const wchar *FileName)
{
  mprintf(L"\n");
  Log(NULL,St(MErrRead),FileName);
  return Ask(St(MRetryAbort))==1;
}


bool uiAskRepeatWrite(const wchar *FileName,bool DiskFull)
{
  mprintf(L"\n");
  Log(NULL,St(DiskFull ? MNotEnoughDisk:MErrWrite),FileName);
  return Ask(St(MRetryAbort))==1;
}


#ifndef SFX_MODULE
const wchar *uiGetMonthName(int Month)
{
  static MSGID MonthID[12]={
         MMonthJan,MMonthFeb,MMonthMar,MMonthApr,MMonthMay,MMonthJun,
         MMonthJul,MMonthAug,MMonthSep,MMonthOct,MMonthNov,MMonthDec
  };
  return St(MonthID[Month]);
}
#endif
