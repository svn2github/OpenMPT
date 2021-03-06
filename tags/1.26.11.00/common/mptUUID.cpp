/*
 * mptUUID.cpp
 * -----------
 * Purpose: UUID utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "mptUUID.h"

#include "mptRandom.h"
#include "mptStringFormat.h"
#include "Endianness.h"

#include <cstdlib>

#if MPT_OS_WINDOWS
#include <windows.h>
#include <rpc.h>
#if defined(MODPLUG_TRACKER) || !defined(NO_DMO)
#include <objbase.h>
#endif // MODPLUG_TRACKER || !NO_DMO
#endif // MPT_OS_WINDOWS


OPENMPT_NAMESPACE_BEGIN


#if MPT_OS_WINDOWS


namespace Util
{


#if defined(MODPLUG_TRACKER) || !defined(NO_DMO)


std::wstring CLSIDToString(CLSID clsid)
//-------------------------------------
{
	std::wstring str;
	LPOLESTR tmp = nullptr;
	::StringFromCLSID(clsid, &tmp);
	if(tmp)
	{
		str = tmp;
		::CoTaskMemFree(tmp);
		tmp = nullptr;
	}
	return str;
}


CLSID StringToCLSID(const std::wstring &str)
//------------------------------------------
{
	CLSID clsid = CLSID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	if(::CLSIDFromString(&tmp[0], &clsid) != S_OK)
	{
		return CLSID();
	}
	return clsid;
}


bool VerifyStringToCLSID(const std::wstring &str, CLSID &clsid)
//-------------------------------------------------------------
{
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	return (::CLSIDFromString(&tmp[0], &clsid) == S_OK);
}


bool IsCLSID(const std::wstring &str)
//-----------------------------------
{
	CLSID clsid = CLSID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	return (::CLSIDFromString(&tmp[0], &clsid) == S_OK);
}


std::wstring IIDToString(IID iid)
//-------------------------------
{
	std::wstring str;
	LPOLESTR tmp = nullptr;
	::StringFromIID(iid, &tmp);
	if(tmp)
	{
		str = tmp;
		::CoTaskMemFree(tmp);
		tmp = nullptr;
	}
	return str;
}


IID StringToIID(const std::wstring &str)
//--------------------------------------
{
	IID iid = IID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	::IIDFromString(&tmp[0], &iid);
	return iid;
}


std::wstring GUIDToString(GUID guid)
//----------------------------------
{
	std::vector<OLECHAR> tmp(256);
	::StringFromGUID2(guid, &tmp[0], static_cast<int>(tmp.size()));
	return &tmp[0];
}


GUID StringToGUID(const std::wstring &str)
//----------------------------------------
{
	return StringToIID(str);
}


GUID CreateGUID()
//---------------
{
	GUID guid = GUID();
	if(::CoCreateGuid(&guid) != S_OK)
	{
		return GUID();
	}
	return guid;
}


UUID StringToUUID(const mpt::ustring &str)
//----------------------------------------
{
	UUID uuid = UUID();
	std::wstring wstr = mpt::ToWide(str);
	std::vector<wchar_t> tmp(wstr.c_str(), wstr.c_str() + wstr.length() + 1);
	if(::UuidFromStringW((RPC_WSTR)(&(tmp[0])), &uuid) != RPC_S_OK)
	{
		return UUID();
	}
	return uuid;
}


mpt::ustring UUIDToString(UUID uuid)
//----------------------------------
{
	std::wstring wstr;
	RPC_WSTR tmp = nullptr;
	if(::UuidToStringW(&uuid, &tmp) != RPC_S_OK)
	{
		return mpt::ustring();
	}
	wstr = (wchar_t*)tmp;
	::RpcStringFreeW(&tmp);
	return mpt::ToUnicode(wstr);
}


bool IsValid(UUID uuid)
//---------------------
{
	return false
		|| uuid.Data1 != 0
		|| uuid.Data2 != 0
		|| uuid.Data3 != 0
		|| uuid.Data4[0] != 0
		|| uuid.Data4[1] != 0
		|| uuid.Data4[2] != 0
		|| uuid.Data4[3] != 0
		|| uuid.Data4[4] != 0
		|| uuid.Data4[5] != 0
		|| uuid.Data4[6] != 0
		|| uuid.Data4[7] != 0
		;
}


#endif // MODPLUG_TRACKER || !NO_DMO


} // namespace Util


#endif // MPT_OS_WINDOWS


namespace mpt
{

#if MPT_OS_WINDOWS

mpt::UUID UUIDFromWin32(::UUID uuid)
{
	return mpt::UUID
		( uuid.Data1
		, uuid.Data2
		, uuid.Data3
		, (static_cast<uint64>(0)
			| (static_cast<uint64>(uuid.Data4[0]) << 56)
			| (static_cast<uint64>(uuid.Data4[1]) << 48)
			| (static_cast<uint64>(uuid.Data4[2]) << 40)
			| (static_cast<uint64>(uuid.Data4[3]) << 32)
			| (static_cast<uint64>(uuid.Data4[4]) << 24)
			| (static_cast<uint64>(uuid.Data4[5]) << 16)
			| (static_cast<uint64>(uuid.Data4[6]) <<  8)
			| (static_cast<uint64>(uuid.Data4[7]) <<  0)
			)
		);
}

::UUID UUIDToWin32(mpt::UUID uuid)
{
	::UUID result = ::UUID();
	result.Data1 = uuid.GetData1();
	result.Data2 = uuid.GetData2();
	result.Data3 = uuid.GetData3();
	result.Data4[0] = static_cast<uint8>(uuid.GetData4() >> 56);
	result.Data4[1] = static_cast<uint8>(uuid.GetData4() >> 48);
	result.Data4[2] = static_cast<uint8>(uuid.GetData4() >> 40);
	result.Data4[3] = static_cast<uint8>(uuid.GetData4() >> 32);
	result.Data4[4] = static_cast<uint8>(uuid.GetData4() >> 24);
	result.Data4[5] = static_cast<uint8>(uuid.GetData4() >> 16);
	result.Data4[6] = static_cast<uint8>(uuid.GetData4() >>  8);
	result.Data4[7] = static_cast<uint8>(uuid.GetData4() >>  0);
	return result;
}

#if defined(MODPLUG_TRACKER)

UUID::UUID(::UUID uuid)
	: Data1(0)
	, Data2(0)
	, Data3(0)
	, Data4(0)
{
	*this = UUIDFromWin32(uuid);
}

UUID::operator ::UUID () const
{
	return UUIDToWin32(*this);
}

#endif // MODPLUG_TRACKER

#endif // MPT_OS_WINDOWS

UUID UUID::Generate()
{
	#if MPT_OS_WINDOWS
		::UUID uuid = ::UUID();
		RPC_STATUS status = ::UuidCreate(&uuid);
		if(status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
		{
			return mpt::UUID::RFC4122Random();
		}
		status = RPC_S_OK;
		if(UuidIsNil(&uuid, &status) != FALSE)
		{
			return mpt::UUID::RFC4122Random();
		}
		if(status != RPC_S_OK)
		{
			return mpt::UUID::RFC4122Random();
		}
		return mpt::UUIDFromWin32(uuid);
	#else
		return RFC4122Random();
	#endif
}

UUID UUID::GenerateLocalUseOnly()
{
	#if MPT_OS_WINDOWS
		#if _WIN32_WINNT >= 0x0501
			// Available since Win2000, but we check for WinXP in order to not use this
			// function in Win32old builds. It is not available on some non-fully
			// patched Win98SE installs in the wild.
			::UUID uuid = ::UUID();
			RPC_STATUS status = ::UuidCreateSequential(&uuid);
			if(status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
			{
				return Generate();
			}
			status = RPC_S_OK;
			if(UuidIsNil(&uuid, &status) != FALSE)
			{
				return mpt::UUID::RFC4122Random();
			}
			if(status != RPC_S_OK)
			{
				return mpt::UUID::RFC4122Random();
			}
			return mpt::UUIDFromWin32(uuid);
		#else
			// Fallback to ::UuidCreate is safe as ::UuidCreateSequential is only a
			// tiny performance optimization.
			return Generate();
		#endif
	#else
		return RFC4122Random();
	#endif
}

UUID UUID::RFC4122Random()
{
	UUID result;
	mpt::thread_safe_prng<mpt::best_prng> & prng = mpt::global_prng();
	result.Data1 = mpt::random<uint32>(prng);
	result.Data2 = mpt::random<uint16>(prng);
	result.Data3 = mpt::random<uint16>(prng);
	result.Data4 = mpt::random<uint64>(prng);
	result.MakeRFC4122(4);
	return result;
}

uint32 UUID::GetData1() const
{
	return Data1;
}

uint16 UUID::GetData2() const
{
	return Data2;
}

uint16 UUID::GetData3() const
{
	return Data3;
}

uint64 UUID::GetData4() const
{
	return Data4;
}

bool UUID::IsNil() const
{
	return (Data1 == 0) && (Data2 == 0) && (Data3 == 0) && (Data4 == 0);
}

bool UUID::IsValid() const
{
	return (Data1 != 0) || (Data2 != 0) || (Data3 != 0) || (Data4 != 0);
}

uint8 UUID::Mm() const
{
	return static_cast<uint8>((Data3 >> 8) & 0xffu);
}

uint8 UUID::Nn() const
{
	return static_cast<uint8>((Data4 >> 56) & 0xffu);
}

uint8 UUID::Variant() const
{
	return Nn() >> 4u;
}

uint8 UUID::Version() const
{
	return Mm() >> 4u;
}

bool UUID::IsRFC4122() const
{
	return (Variant() & 0xcu) == 0x8u;
}

void UUID::MakeRFC4122(uint8 version)
{
	// variant
	uint8 Nn = static_cast<uint8>((Data4 >> 56) & 0xffu);
	Data4 &= 0x00ffffffffffffffull;
	Nn &= ~(0xc0u);
	Nn |= 0x80u;
	Data4 |= static_cast<uint64>(Nn) << 56;
	// version
	version &= 0x0fu;
	uint8 Mm = static_cast<uint8>((Data3 >> 8) & 0xffu);
	Data3 &= 0x00ffu;
	Mm &= ~(0xf0u);
	Mm |= (version << 4u);
	Data3 |= static_cast<uint16>(Mm) << 8;
}

void UUID::ConvertEndianness()
{
	SwapBytesBE(Data1);
	SwapBytesBE(Data2);
	SwapBytesBE(Data3);
	SwapBytesBE(Data4);
}

UUID::UUID()
	: Data1(0)
	, Data2(0)
	, Data3(0)
	, Data4(0)
{
	return;
}

UUID::UUID(uint32 Data1, uint16 Data2, uint16 Data3, uint64 Data4)
	: Data1(Data1)
	, Data2(Data2)
	, Data3(Data3)
	, Data4(Data4)
{
	return;
}

bool operator==(const mpt::UUID & a, const mpt::UUID & b)
{
	return (a.Data1 == b.Data1) && (a.Data2 == b.Data2) && (a.Data3 == b.Data3) && (a.Data4 == b.Data4);
}

bool operator!=(const mpt::UUID & a, const mpt::UUID & b)
{
	return (a.Data1 != b.Data1) || (a.Data2 != b.Data2) || (a.Data3 != b.Data3) || (a.Data4 != b.Data4);
}

UUID UUID::FromString(const mpt::ustring &str)
{
	std::vector<mpt::ustring> segments = mpt::String::Split<mpt::ustring>(str, MPT_USTRING("-"));
	if(segments.size() != 5)
	{
		return UUID();
	}
	if(segments[0].length() != 8)
	{
		return UUID();
	}
	if(segments[1].length() != 4)
	{
		return UUID();
	}
	if(segments[2].length() != 4)
	{
		return UUID();
	}
	if(segments[3].length() != 4)
	{
		return UUID();
	}
	if(segments[4].length() != 12)
	{
		return UUID();
	}
	UUID result;
	result.Data1 = mpt::String::Parse::Hex<uint32>(segments[0]);
	result.Data2 = mpt::String::Parse::Hex<uint16>(segments[1]);
	result.Data3 = mpt::String::Parse::Hex<uint16>(segments[2]);
	result.Data4 = mpt::String::Parse::Hex<uint64>(segments[3] + segments[4]);
	return result;
}

mpt::ustring UUID::ToUString() const
{
	return mpt::ustring()
		+ mpt::ufmt::hex0<8>(Data1)
		+ MPT_USTRING("-")
		+ mpt::ufmt::hex0<4>(Data2)
		+ MPT_USTRING("-")
		+ mpt::ufmt::hex0<4>(Data3)
		+ MPT_USTRING("-")
		+ mpt::ufmt::hex0<4>(static_cast<uint16>(Data4 >> 48))
		+ MPT_USTRING("-")
		+ mpt::ufmt::hex0<4>(static_cast<uint16>(Data4 >> 32))
		+ mpt::ufmt::hex0<8>(static_cast<uint32>(Data4 >>  0))
		;
}

} // namespace mpt


OPENMPT_NAMESPACE_END
