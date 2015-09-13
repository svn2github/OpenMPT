#include "rar.hpp"

static int SleepTime=0;

void InitSystemOptions(int SleepTime)
{
  ::SleepTime=SleepTime;
}


#if !defined(SFX_MODULE) && !defined(SETUP)
void SetPriority(int Priority)
{
#ifdef _WIN_ALL
  uint PriorityClass;
  int PriorityLevel;
  if (Priority<1 || Priority>15)
    return;

  if (Priority==1)
  {
    PriorityClass=IDLE_PRIORITY_CLASS;
    PriorityLevel=THREAD_PRIORITY_IDLE;

//  Background mode for Vista, can be slow for many small files.
//    if (WinNT()>=WNT_VISTA)
//      SetPriorityClass(GetCurrentProcess(),PROCESS_MODE_BACKGROUND_BEGIN);
  }
  else
    if (Priority<7)
    {
      PriorityClass=IDLE_PRIORITY_CLASS;
      PriorityLevel=Priority-4;
    }
    else
      if (Priority==7)
      {
        PriorityClass=BELOW_NORMAL_PRIORITY_CLASS;
        PriorityLevel=THREAD_PRIORITY_ABOVE_NORMAL;
      }
      else
        if (Priority<10)
        {
          PriorityClass=NORMAL_PRIORITY_CLASS;
          PriorityLevel=Priority-7;
        }
        else
          if (Priority==10)
          {
            PriorityClass=ABOVE_NORMAL_PRIORITY_CLASS;
            PriorityLevel=THREAD_PRIORITY_NORMAL;
          }
          else
          {
            PriorityClass=HIGH_PRIORITY_CLASS;
            PriorityLevel=Priority-13;
          }
  SetPriorityClass(GetCurrentProcess(),PriorityClass);
  SetThreadPriority(GetCurrentThread(),PriorityLevel);

#ifdef RAR_SMP
  ThreadPool::SetPriority(PriorityLevel);
#endif

#endif
}
#endif


// Monotonic clock. Like clock(), returns time passed in CLOCKS_PER_SEC items.
// In Android 5+ and Unix usual clock() returns time spent by all threads
// together, so we cannot use it to measure time intervals anymore.
clock_t MonoClock()
{
#if defined(_ANDROID) && defined(_UNIX) && defined(CLOCK_MONOTONIC)
  struct timespec CurTime;
  clock_gettime(CLOCK_MONOTONIC, &CurTime);
  int64 nsec = int64(CurTime.tv_sec)*1000000000 + CurTime.tv_nsec;
  nsec /= 1000000000 / CLOCKS_PER_SEC;
  return (clock_t)nsec;
#else
  return clock();
#endif
}



#ifndef SETUP
void Wait()
{
  if (ErrHandler.UserBreak)
    ErrHandler.Exit(RARX_USERBREAK);
#if defined(_WIN_ALL) && !defined(SFX_MODULE)
  if (SleepTime!=0)
  {
    static clock_t LastTime=MonoClock();
    if (MonoClock()-LastTime>10*CLOCKS_PER_SEC/1000)
    {
      Sleep(SleepTime);
      LastTime=MonoClock();
    }
  }
#endif
#ifdef _WIN_ALL
  // Reset system sleep timer to prevent system going sleep.
  SetThreadExecutionState(ES_SYSTEM_REQUIRED);
#endif
}
#endif




#if defined(_WIN_ALL) && !defined(SFX_MODULE) && !defined(SHELL_EXT) && !defined(SETUP)
void Shutdown()
{
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;
  if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
  {
    LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES)NULL,0);
  }
  ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE|EWX_POWEROFF,SHTDN_REASON_FLAG_PLANNED);
}
#endif



#ifdef USE_SSE
SSE_VERSION _SSE_Version=GetSSEVersion();

SSE_VERSION GetSSEVersion()
{
  int CPUInfo[4];
  __cpuid(CPUInfo, 1);
  if ((CPUInfo[2] & 0x80000)!=0)
    return SSE_SSE41;
  if ((CPUInfo[2] & 0x200)!=0)
    return SSE_SSSE3;
  if ((CPUInfo[3] & 0x4000000)!=0)
    return SSE_SSE2;
  if ((CPUInfo[3] & 0x2000000)!=0)
    return SSE_SSE;
  return SSE_NONE;
}
#endif
