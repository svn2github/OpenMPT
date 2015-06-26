/*
 * SoundDeviceManager.cpp
 * ----------------------
 * Purpose: Sound device manager class.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "SoundDeviceManager.h"
#include "SoundDevice.h"

#include "SoundDeviceASIO.h"
#include "SoundDeviceDirectSound.h"
#include "SoundDevicePortAudio.h"
#include "SoundDeviceWaveout.h"


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice {

	
struct CompareInfo
{
	std::map<SoundDevice::Type, int> ordering;
	CompareInfo(const std::map<SoundDevice::Type, int> &ordering)
		: ordering(ordering)
	{
		return;
	}
	bool operator () (const SoundDevice::Info &x, const SoundDevice::Info &y)
	{
		return (ordering[x.type] > ordering[y.type])
			|| ((ordering[x.type] == ordering[y.type]) && (x.isDefault && !y.isDefault))
			;
	}
};


template <typename Tdevice>
void Manager::EnumerateDevices()
//------------------------------
{
	const std::vector<SoundDevice::Info> infos = Tdevice::EnumerateDevices();
	m_SoundDevices.insert(m_SoundDevices.end(), infos.begin(), infos.end());
	for(std::vector<SoundDevice::Info>::const_iterator it = infos.begin(); it != infos.end(); ++it)
	{
		SoundDevice::Identifier identifier = it->GetIdentifier();
		if(!identifier.empty())
		{
			m_DeviceFactoryMethods[identifier] = ConstructSoundDevice<Tdevice>;
		}
	}
}


template <typename Tdevice>
SoundDevice::IBase* Manager::ConstructSoundDevice(const SoundDevice::Info &info)
//------------------------------------------------------------------------------
{
	return new Tdevice(info);
}


void Manager::ReEnumerate()
//-------------------------
{
	m_SoundDevices.clear();
	m_DeviceUnavailable.clear();
	m_DeviceFactoryMethods.clear();
	m_DeviceCaps.clear();
	m_DeviceDynamicCaps.clear();

#ifndef NO_PORTAUDIO
	if(IsComponentAvailable(m_PortAudio))
	{
		m_PortAudio->ReInit();
	}
#endif // NO_PORTAUDIO

#if MPT_OS_WINDOWS
	if(IsComponentAvailable(m_WaveOut))
	{
		EnumerateDevices<CWaveDevice>();
	}
#endif // MPT_OS_WINDOWS

#ifndef NO_DSOUND
	// kind of deprecated by now
	if(IsComponentAvailable(m_DirectSound))
	{
		EnumerateDevices<CDSoundDevice>();
	}
#endif // NO_DSOUND

#ifndef NO_ASIO
	if(IsComponentAvailable(m_ASIO))
	{
		EnumerateDevices<CASIODevice>();
	}
#endif // NO_ASIO

#ifndef NO_PORTAUDIO
	if(IsComponentAvailable(m_PortAudio))
	{
		EnumerateDevices<CPortaudioDevice>();
	}
#endif // NO_PORTAUDIO

	std::map<SoundDevice::Type, int> typePriorities;
	if(mpt::Windows::Version::IsWine())
	{ // Wine
		typePriorities[SoundDevice::TypeDSOUND] = 29;
		typePriorities[SoundDevice::TypeWAVEOUT] = 28;
		typePriorities[SoundDevice::TypePORTAUDIO_WASAPI] = 27;
		typePriorities[SoundDevice::TypeASIO] = 21;
		typePriorities[SoundDevice::TypePORTAUDIO_WMME] = 19;
		typePriorities[SoundDevice::TypePORTAUDIO_DS] = 18;
		typePriorities[SoundDevice::TypePORTAUDIO_WDMKS] = -1;
	} else if(mpt::Windows::Version::Is9x())
	{ // Win9x
		typePriorities[SoundDevice::TypeWAVEOUT] = 29;
		typePriorities[SoundDevice::TypeDSOUND] = 28;
		typePriorities[SoundDevice::TypePORTAUDIO_WMME] = 19;
		typePriorities[SoundDevice::TypePORTAUDIO_DS] = 18;
		typePriorities[SoundDevice::TypeASIO] = 1;
		typePriorities[SoundDevice::TypePORTAUDIO_WDMKS] = -1;
		typePriorities[SoundDevice::TypePORTAUDIO_WASAPI] = -2;
	} else if(mpt::Windows::Version::IsBefore(mpt::Windows::Version::WinVista))
	{ // WinXP
		typePriorities[SoundDevice::TypeWAVEOUT] = 29;
		typePriorities[SoundDevice::TypeASIO] = 28;
		typePriorities[SoundDevice::TypeDSOUND] = 27;
		typePriorities[SoundDevice::TypePORTAUDIO_WDMKS] = 26;
		typePriorities[SoundDevice::TypePORTAUDIO_WMME] = 19;
		typePriorities[SoundDevice::TypePORTAUDIO_DS] = 17;
		typePriorities[SoundDevice::TypePORTAUDIO_WASAPI] = -1;
	} else
	{ // >=Vista
		typePriorities[SoundDevice::TypeWAVEOUT] = 29;
		typePriorities[SoundDevice::TypePORTAUDIO_WASAPI] = 28;
		typePriorities[SoundDevice::TypeASIO] = 27;
		typePriorities[SoundDevice::TypePORTAUDIO_WDMKS] = 26;
		typePriorities[SoundDevice::TypePORTAUDIO_WMME] = 19;
		typePriorities[SoundDevice::TypeDSOUND] = -1;
		typePriorities[SoundDevice::TypePORTAUDIO_DS] = -2;
	}
	std::stable_sort(m_SoundDevices.begin(), m_SoundDevices.end(), CompareInfo(typePriorities));

	MPT_LOG(LogDebug, "sounddev", MPT_USTRING("Sound Devices enumerated:"));
	for(std::size_t i = 0; i < m_SoundDevices.size(); ++i)
	{
		MPT_LOG(LogDebug, "sounddev", MPT_UFORMAT(" Identifier : %1", m_SoundDevices[i].GetIdentifier()));
		MPT_LOG(LogDebug, "sounddev", MPT_UFORMAT("  Type      : %1", m_SoundDevices[i].type));
		MPT_LOG(LogDebug, "sounddev", MPT_UFORMAT("  InternalID: %1", m_SoundDevices[i].internalID));
		MPT_LOG(LogDebug, "sounddev", MPT_UFORMAT("  API Name  : %1", m_SoundDevices[i].apiName));
		MPT_LOG(LogDebug, "sounddev", MPT_UFORMAT("  Name      : %1", m_SoundDevices[i].name));
		for(std::map<mpt::ustring, mpt::ustring>::const_iterator extraIt = m_SoundDevices[i].extraData.begin(); extraIt != m_SoundDevices[i].extraData.end(); ++extraIt)
		{
			Log(LogDebug, mpt::String::Print(MPT_USTRING("  Extra Data: %1 = %2"), extraIt->first, extraIt->second));
		}
	}
	
}


SoundDevice::Manager::GlobalID Manager::GetGlobalID(SoundDevice::Identifier identifier) const
//-------------------------------------------------------------------------------------------
{
	for(std::size_t i = 0; i < m_SoundDevices.size(); ++i)
	{
		if(m_SoundDevices[i].GetIdentifier() == identifier)
		{
			return i;
		}
	}
	return ~SoundDevice::Manager::GlobalID();
}


SoundDevice::Info Manager::FindDeviceInfo(SoundDevice::Manager::GlobalID id) const
//--------------------------------------------------------------------------------
{
	if(id > m_SoundDevices.size())
	{
		return SoundDevice::Info();
	}
	return m_SoundDevices[id];
}


SoundDevice::Info Manager::FindDeviceInfo(SoundDevice::Identifier identifier) const
//---------------------------------------------------------------------------------
{
	if(m_SoundDevices.empty())
	{
		return SoundDevice::Info();
	}
	if(identifier.empty())
	{
		return SoundDevice::Info();
	}
	for(std::vector<SoundDevice::Info>::const_iterator it = begin(); it != end(); ++it)
	{
		if(it->GetIdentifier() == identifier)
		{
			return *it;
		}
	}
	return SoundDevice::Info();
}


SoundDevice::Info Manager::FindDeviceInfoBestMatch(SoundDevice::Identifier identifier, bool preferSameType)
//---------------------------------------------------------------------------------------------------------
{
	if(m_SoundDevices.empty())
	{
		return SoundDevice::Info();
	}
	if(!identifier.empty())
	{ // valid identifier
		for(std::vector<SoundDevice::Info>::const_iterator it = begin(); it != end(); ++it)
		{
			if((it->GetIdentifier() == identifier) && !IsDeviceUnavailable(it->GetIdentifier()))
			{ // exact match
				return *it;
			}
		}
		const SoundDevice::Type type = ParseType(identifier);
		if(type == TypePORTAUDIO_WASAPI)
		{
			// WASAPI devices might change names if a different connector jack is used.
			// In order to avoid defaulting to wave mapper in that case,
			// just find the first WASAPI device.
			preferSameType = true;
		}
		if(preferSameType)
		{ // find first avilable device of given type
			for(std::vector<SoundDevice::Info>::const_iterator it = begin(); it != end(); ++it)
			{
				if((it->type == type) && !IsDeviceUnavailable(it->GetIdentifier()))
				{
					return *it;
				}
			}
		}
	}
	for(std::vector<SoundDevice::Info>::const_iterator it = begin(); it != end(); ++it)
	{ // find first available device
		if(!IsDeviceUnavailable(it->GetIdentifier()))
		{
			return *it;
		}
	}
	// default to first device
	return *begin();
}


bool Manager::OpenDriverSettings(SoundDevice::Identifier identifier, SoundDevice::IMessageReceiver *messageReceiver, SoundDevice::IBase *currentSoundDevice)
//----------------------------------------------------------------------------------------------------------------------------------------------------------
{
	bool result = false;
	if(currentSoundDevice && FindDeviceInfo(identifier).IsValid() && (currentSoundDevice->GetDeviceInfo().GetIdentifier() == identifier))
	{
		result = currentSoundDevice->OpenDriverSettings();
	} else
	{
		SoundDevice::IBase *dummy = CreateSoundDevice(identifier);
		if(dummy)
		{
			dummy->SetMessageReceiver(messageReceiver);
			result = dummy->OpenDriverSettings();
		}
		delete dummy;
	}
	return result;
}


SoundDevice::Caps Manager::GetDeviceCaps(SoundDevice::Identifier identifier, SoundDevice::IBase *currentSoundDevice)
//------------------------------------------------------------------------------------------------------------------
{
	if(m_DeviceCaps.find(identifier) == m_DeviceCaps.end())
	{
		if(currentSoundDevice && FindDeviceInfo(identifier).IsValid() && (currentSoundDevice->GetDeviceInfo().GetIdentifier() == identifier))
		{
			m_DeviceCaps[identifier] = currentSoundDevice->GetDeviceCaps();
		} else
		{
			SoundDevice::IBase *dummy = CreateSoundDevice(identifier);
			if(dummy)
			{
				m_DeviceCaps[identifier] = dummy->GetDeviceCaps();
			} else
			{
				SetDeviceUnavailable(identifier);
			}
			delete dummy;
		}
	}
	return m_DeviceCaps[identifier];
}


SoundDevice::DynamicCaps Manager::GetDeviceDynamicCaps(SoundDevice::Identifier identifier, const std::vector<uint32> &baseSampleRates, SoundDevice::IMessageReceiver *messageReceiver, SoundDevice::IBase *currentSoundDevice, bool update)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	if((m_DeviceDynamicCaps.find(identifier) == m_DeviceDynamicCaps.end()) || update)
	{
		if(currentSoundDevice && FindDeviceInfo(identifier).IsValid() && (currentSoundDevice->GetDeviceInfo().GetIdentifier() == identifier))
		{
			m_DeviceDynamicCaps[identifier] = currentSoundDevice->GetDeviceDynamicCaps(baseSampleRates);
			if(!currentSoundDevice->IsAvailable())
			{
				SetDeviceUnavailable(identifier);
			}
		} else
		{
			SoundDevice::IBase *dummy = CreateSoundDevice(identifier);
			if(dummy)
			{
				dummy->SetMessageReceiver(messageReceiver);
				m_DeviceDynamicCaps[identifier] = dummy->GetDeviceDynamicCaps(baseSampleRates);
				if(!dummy->IsAvailable())
				{
					SetDeviceUnavailable(identifier);
				}
			} else
			{
				SetDeviceUnavailable(identifier);
			}
			delete dummy;
		}
	}
	return m_DeviceDynamicCaps[identifier];
}


SoundDevice::IBase * Manager::CreateSoundDevice(SoundDevice::Identifier identifier)
//---------------------------------------------------------------------------------
{
	const SoundDevice::Info info = FindDeviceInfo(identifier);
	if(!info.IsValid())
	{
		return nullptr;
	}
	if(m_DeviceFactoryMethods.find(identifier) == m_DeviceFactoryMethods.end())
	{
		return nullptr;
	}
	if(!m_DeviceFactoryMethods[identifier])
	{
		return nullptr;
	}
	SoundDevice::IBase *result = m_DeviceFactoryMethods[identifier](info);
	if(!result)
	{
		return nullptr;
	}
	if(!result->Init(m_AppInfo))
	{
		delete result;
		result = nullptr;
		return nullptr;
	}
	m_DeviceCaps[identifier] = result->GetDeviceCaps(); // update cached caps
	return result;
}


Manager::Manager(SoundDevice::AppInfo appInfo)
//--------------------------------------------
	: m_AppInfo(appInfo)
{
	ReEnumerate();
}


Manager::~Manager()
//-----------------
{
	return;
}


namespace Legacy
{
SoundDevice::Info FindDeviceInfo(SoundDevice::Manager &manager, SoundDevice::Legacy::ID id)
{
	if(manager.GetDeviceInfos().empty())
	{
		return SoundDevice::Info();
	}
	SoundDevice::Type type = SoundDevice::Type();
	switch(id & SoundDevice::Legacy::MaskType)
	{
		case SoundDevice::Legacy::TypeWAVEOUT:
			type = SoundDevice::TypeWAVEOUT;
			break;
		case SoundDevice::Legacy::TypeDSOUND:
			type = SoundDevice::TypeDSOUND;
			break;
		case SoundDevice::Legacy::TypeASIO:
			type = SoundDevice::TypeASIO;
			break;
		case SoundDevice::Legacy::TypePORTAUDIO_WASAPI:
			type = SoundDevice::TypePORTAUDIO_WASAPI;
			break;
		case SoundDevice::Legacy::TypePORTAUDIO_WDMKS:
			type = SoundDevice::TypePORTAUDIO_WDMKS;
			break;
		case SoundDevice::Legacy::TypePORTAUDIO_WMME:
			type = SoundDevice::TypePORTAUDIO_WMME;
			break;
		case SoundDevice::Legacy::TypePORTAUDIO_DS:
			type = SoundDevice::TypePORTAUDIO_DS;
			break;
	}
	if(type.empty())
	{	// fallback to first device
		return *manager.begin();
	}
	std::size_t index = static_cast<uint8>(id & SoundDevice::Legacy::MaskIndex);
	std::size_t seenDevicesOfDesiredType = 0;
	for(std::vector<SoundDevice::Info>::const_iterator it = manager.begin(); it != manager.end(); ++it)
	{
		if(it->type == type)
		{
			if(seenDevicesOfDesiredType == index)
			{
				if(!it->IsValid())
				{	// fallback to first device
					return *manager.begin();
				}
				return *it;
			}
			seenDevicesOfDesiredType++;
		}
	}
	// default to first device
	return *manager.begin();
}
}


} // namespace SoundDevice


OPENMPT_NAMESPACE_END
