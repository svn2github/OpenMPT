/*
 * mutex.h
 * -------
 * Purpose: Partially implement c++ mutexes as far as openmpt needs them. Can eventually go away when we only support c++11 compilers some time.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#ifdef MODPLUG_TRACKER
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <windows.h>
#else
#include <pthread.h>
#endif
#endif // MODPLUG_TRACKER

OPENMPT_NAMESPACE_BEGIN

#ifdef MODPLUG_TRACKER

namespace Util {

#ifdef _WIN32

// compatible with c++11 std::mutex, can eventually be replaced without touching any usage site
class mutex {
private:
	CRITICAL_SECTION impl;
public:
	mutex() { InitializeCriticalSection(&impl); }
	~mutex() { DeleteCriticalSection(&impl); }
	void lock() { EnterCriticalSection(&impl); }
	void unlock() { LeaveCriticalSection(&impl); }
};

// compatible with c++11 std::recursive_mutex, can eventually be replaced without touching any usage site
class recursive_mutex {
private:
	CRITICAL_SECTION impl;
public:
	recursive_mutex() { InitializeCriticalSection(&impl); }
	~recursive_mutex() { DeleteCriticalSection(&impl); }
	void lock() { EnterCriticalSection(&impl); }
	void unlock() { LeaveCriticalSection(&impl); }
};

#else // !_WIN32

class mutex {
private:
	pthread_mutex_t hLock;
public:
	mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
		pthread_mutex_init(&hLock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	~mutex() { pthread_mutex_destroy(&hLock); }
	void lock() { pthread_mutex_lock(&hLock); }
	void unlock() { pthread_mutex_unlock(&hLock); }
};

class recursive_mutex {
private:
	pthread_mutex_t hLock;
public:
	recursive_mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&hLock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	~recursive_mutex() { pthread_mutex_destroy(&hLock); }
	void lock() { pthread_mutex_lock(&hLock); }
	void unlock() { pthread_mutex_unlock(&hLock); }
};

#endif // _WIN32

// compatible with c++11 std::lock_guard, can eventually be replaced without touching any usage site
template< typename mutex_type >
class lock_guard {
private:
	mutex_type & mutex;
public:
	lock_guard( mutex_type & m ) : mutex(m) { mutex.lock(); }
	~lock_guard() { mutex.unlock(); }
};

} // namespace Util

#endif // MODPLUG_TRACKER

OPENMPT_NAMESPACE_END
