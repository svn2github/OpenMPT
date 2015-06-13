/*
 * SoundDevicePortAudio.h
 * ----------------------
 * Purpose: PortAudio sound device driver class.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "SoundDeviceBase.h"

#include "../common/ComponentManager.h"

#ifndef NO_PORTAUDIO
#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT)
#include "portaudio/include/portaudio.h"
#if MPT_OS_WINDOWS
#include "portaudio/include/pa_win_wasapi.h"
#endif // MPT_OS_WINDOWS
#else
#include <portaudio.h>
#endif
#endif

OPENMPT_NAMESPACE_BEGIN

namespace SoundDevice {

#ifndef NO_PORTAUDIO

//=========================================
class CPortaudioDevice: public SoundDevice::Base
//=========================================
{

protected:

	PaDeviceIndex m_DeviceIndex;
	PaHostApiTypeId m_HostApiType;
	PaStreamParameters m_StreamParameters;
	PaStreamParameters m_InputStreamParameters;
#if MPT_OS_WINDOWS
	PaWasapiStreamInfo m_WasapiStreamInfo;
#endif // MPT_OS_WINDOWS
	PaStream * m_Stream;
	const PaStreamInfo * m_StreamInfo;
	void * m_CurrentFrameBuffer;
	const void * m_CurrentFrameBufferInput;
	unsigned long m_CurrentFrameCount;

	double m_CurrentRealLatency; // seconds
	mpt::atomic_uint32_t m_StatisticPeriodFrames;

public:

	CPortaudioDevice(SoundDevice::Info info);
	~CPortaudioDevice();

public:

	bool InternalOpen();
	bool InternalClose();
	void InternalFillAudioBuffer();
	bool InternalStart();
	void InternalStop();
	bool InternalIsOpen() const { return m_Stream ? true : false; }
	bool InternalHasGetStreamPosition() const { return false; }
	int64 InternalGetStreamPositionFrames() const;
	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;
	SoundDevice::Statistics GetStatistics() const;
	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);
	bool OpenDriverSettings();

	int StreamCallback(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags
		);

public:

	static int StreamCallbackWrapper(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData
		);

	static std::vector<SoundDevice::Info> EnumerateDevices();

private:

	bool HasInputChannelsOnSameDevice() const;

	static std::vector<std::pair<PaDeviceIndex, mpt::ustring> > EnumerateInputOnlyDevices(PaHostApiTypeId hostApiType);

};


class ComponentPortAudio : public ComponentBuiltin
{
	MPT_DECLARE_COMPONENT_MEMBERS
public:
	ComponentPortAudio();
	virtual ~ComponentPortAudio();
	std::string GetSettingsKey() const { return "PortAudio"; }
	virtual bool DoInitialize();
	bool ReInit();
};


#endif // NO_PORTAUDIO


} // namespace SoundDevice


OPENMPT_NAMESPACE_END
