/*
 * SoundDevice.h
 * -------------
 * Purpose: Sound device interface class.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "../common/mutex.h"
#include "../soundlib/SampleFormat.h"

#include <map>
#include <vector>


class ISoundDevice;
class IFillAudioBuffer;
class ISoundSource;


////////////////////////////////////////////////////////////////////////////////////
//
// ISoundSource: provides streaming audio data to a device
//


struct SoundDeviceSettings;


//====================
class IFillAudioBuffer
//====================
{
public:
	virtual void FillAudioBuffer() = 0;
};


//=========================
class ISoundMessageReceiver
//=========================
{
public:
	virtual void AudioMessage(const std::string &str) = 0;
};


//================
class ISoundSource
//================
{
public:
	virtual void FillAudioBufferLocked(IFillAudioBuffer &callback) = 0; // take any locks needed while rendering audio and then call FillAudioBuffer
	virtual void AudioRead(const SoundDeviceSettings &settings, std::size_t numFrames, void *buffer) = 0;
	virtual void AudioDone(const SoundDeviceSettings &settings, std::size_t numFrames, int64 streamPosition) = 0; // in sample frames
};


////////////////////////////////////////////////////////////////////////////////////
//
// ISoundDevice Interface
//

enum SoundDeviceType
{
	// do not change old values, these get saved to the ini
	SNDDEV_INVALID          =-1,
	SNDDEV_WAVEOUT          = 0,
	SNDDEV_DSOUND           = 1,
	SNDDEV_ASIO             = 2,
	SNDDEV_PORTAUDIO_WASAPI = 3,
	SNDDEV_PORTAUDIO_WDMKS  = 4,
	SNDDEV_PORTAUDIO_WMME   = 5,
	SNDDEV_PORTAUDIO_DS     = 6,
	SNDDEV_PORTAUDIO_ASIO   = 7,
	SNDDEV_NUM_DEVTYPES
};

typedef uint8 SoundDeviceIndex;

template<typename T>
bool SoundDeviceIndexIsValid(const T & x)
{
	return 0 <= x && x <= std::numeric_limits<SoundDeviceIndex>::max();
}

//=================
class SoundDeviceID
//=================
{
private:
	SoundDeviceType type;
	SoundDeviceIndex index;
public:
	SoundDeviceID() : type(SNDDEV_WAVEOUT), index(0) {}
	SoundDeviceID(SoundDeviceType type, SoundDeviceIndex index)
		: type(type)
		, index(index)
	{
		return;
	}
	SoundDeviceType GetType() const { return type; }
	SoundDeviceIndex GetIndex() const { return index; }
	bool operator == (const SoundDeviceID &cmp) const
	{
		return (type == cmp.type) && (index == cmp.index);
	}

	bool operator != (const SoundDeviceID &cmp) const
	{
		return (type != cmp.type) || (index != cmp.index);
	}
	bool operator < (const SoundDeviceID &cmp) const
	{
		return (type < cmp.type) || (type == cmp.type && index < cmp.index);
	}
public:
	// Do not change these. These functions are used to manipulate the value that gets stored in the settings.
	template<typename T>
	static SoundDeviceID FromIdRaw(T id_)
	{
		uint16 id = static_cast<uint16>(id_);
		return SoundDeviceID((SoundDeviceType)((id>>8)&0xff), (id>>0)&0xff);
	}
	uint16 GetIdRaw() const
	{
		return static_cast<uint16>(((int)type<<8) | (index<<0));
	}
};

#define SNDDEV_MAXBUFFERS	       	  256
#define SNDDEV_MAXBUFFERSIZE        (1024*1024)
#define SNDDEV_MINLATENCY_MS        1
#define SNDDEV_MAXLATENCY_MS        500
#define SNDDEV_MINUPDATEINTERVAL_MS 1
#define SNDDEV_MAXUPDATEINTERVAL_MS 200


struct SoundDeviceSettings
{
	HWND hWnd;
	uint32 LatencyMS;
	uint32 UpdateIntervalMS;
	uint32 Samplerate;
	uint8 Channels;
	SampleFormat sampleFormat;
	bool ExclusiveMode; // Use hardware buffers directly
	bool BoostThreadPriority; // Boost thread priority for glitch-free audio rendering
	SoundDeviceSettings()
		: hWnd(NULL)
		, LatencyMS(100)
		, UpdateIntervalMS(5)
		, Samplerate(48000)
		, Channels(2)
		, sampleFormat(SampleFormatInt16)
		, ExclusiveMode(false)
		, BoostThreadPriority(true)
	{
		return;
	}
	bool operator == (const SoundDeviceSettings &cmp) const
	{
		return true
			&& hWnd == cmp.hWnd
			&& LatencyMS == cmp.LatencyMS
			&& UpdateIntervalMS == cmp.UpdateIntervalMS
			&& Samplerate == cmp.Samplerate
			&& Channels == cmp.Channels
			&& sampleFormat == cmp.sampleFormat
			&& ExclusiveMode == cmp.ExclusiveMode
			&& BoostThreadPriority == cmp.BoostThreadPriority
			;
	}
	bool operator != (const SoundDeviceSettings &cmp) const
	{
		return !(*this == cmp);
	}
	std::size_t GetBytesPerFrame() const
	{
		return (sampleFormat.GetBitsPerSample()/8) * Channels;
	}
	std::size_t GetBytesPerSecond() const
	{
		return Samplerate * GetBytesPerFrame();
	}
};


struct SoundDeviceCaps
{
	uint32 currentSampleRate;
	std::vector<uint32> supportedSampleRates;	// Which samplerates are actually supported by the device. Currently only implemented properly for ASIO, DirectSound and PortAudio.
	SoundDeviceCaps()
		: currentSampleRate(0)
	{
		return;
	}
};


class SoundDevicesManager;


//=============================================
class ISoundDevice : protected IFillAudioBuffer
//=============================================
{
	friend class SoundDevicesManager;

private:

	ISoundSource *m_Source;
	ISoundMessageReceiver *m_MessageReceiver;

	const SoundDeviceID m_ID;

	std::wstring m_InternalID;

protected:

	SoundDeviceSettings m_Settings;

private:

	double m_RealLatency;
	double m_RealUpdateInterval;
	int m_RealNumBuffers;

	bool m_IsPlaying;

	mutable Util::mutex m_StreamPositionMutex;
	int64 m_StreamPositionRenderFrames;
	int64 m_StreamPositionOutputFrames;

protected:

	virtual void FillAudioBuffer() = 0;

	void SourceFillAudioBufferLocked();
	void SourceAudioRead(void *buffer, std::size_t numFrames);
	void SourceAudioDone(std::size_t numFrames, int32 framesLatency);

	void AudioSendMessage(const std::string &str);

protected:

	bool FillWaveFormatExtensible(WAVEFORMATEXTENSIBLE &WaveFormat);

	void UpdateLatencyInfo(double latency, double updateInterval, int numBuffers)
	{
		m_RealLatency = latency;
		m_RealUpdateInterval = updateInterval;
		m_RealNumBuffers = numBuffers;
	}

	virtual bool InternalHasGetStreamPosition() const { return false; }
	virtual int64 InternalGetStreamPositionFrames() const { return 0; }

	virtual bool InternalIsOpen() const = 0;

	virtual bool InternalOpen() = 0;
	virtual void InternalStart() = 0;
	virtual void InternalStop() = 0;
	virtual bool InternalClose() = 0;

public:

	ISoundDevice(SoundDeviceID id, const std::wstring &internalID);
	virtual ~ISoundDevice();

	void SetSource(ISoundSource *source) { m_Source = source; }
	ISoundSource *GetSource() const { return m_Source; }
	void SetMessageReceiver(ISoundMessageReceiver *receiver) { m_MessageReceiver = receiver; }
	ISoundMessageReceiver *GetMessageReceiver() const { return m_MessageReceiver; }

	SoundDeviceID GetDeviceID() const { return m_ID; }
	SoundDeviceType GetDeviceType() const { return m_ID.GetType(); }
	SoundDeviceIndex GetDeviceIndex() const { return m_ID.GetIndex(); }
	std::wstring GetDeviceInternalID() const { return m_InternalID; }

	virtual SoundDeviceCaps GetDeviceCaps(const std::vector<uint32> &baseSampleRates);

	bool Open(const SoundDeviceSettings &settings);
	bool Close();
	void Start();
	void Stop();

	bool IsOpen() const { return InternalIsOpen(); }
	bool IsPlaying() const { return m_IsPlaying; }

	SampleFormat GetActualSampleFormat() { return IsOpen() ? m_Settings.sampleFormat : SampleFormatInvalid; }

	double GetRealLatency() const  { return m_RealLatency; } // seconds
	double GetRealUpdateInterval() const { return m_RealUpdateInterval; } // seconds
	int GetRealNumBuffers() const { return m_RealNumBuffers; }

	virtual double GetCurrentRealLatency() const { return GetRealLatency(); }

	int64 GetStreamPositionFrames() const;

};


struct SoundDeviceInfo
{
	SoundDeviceID id;
	std::wstring name;
	std::wstring apiName;
	std::wstring internalID;
	SoundDeviceInfo() { }
	SoundDeviceInfo(SoundDeviceID id, const std::wstring &name, const std::wstring &apiName, const std::wstring &internalID = std::wstring())
		: id(id)
		, name(name)
		, apiName(apiName)
		, internalID(internalID)
	{
		return;
	}
};


//=======================
class SoundDevicesManager
//=======================
{
private:
	std::vector<SoundDeviceInfo> m_SoundDevices;
	std::map<SoundDeviceID, SoundDeviceCaps> m_DeviceCaps;

public:
	SoundDevicesManager();
	~SoundDevicesManager();

public:

	void ReEnumerate();

	std::vector<SoundDeviceInfo>::const_iterator begin() const { return m_SoundDevices.begin(); }
	std::vector<SoundDeviceInfo>::const_iterator end() const { return m_SoundDevices.end(); }
	const std::vector<SoundDeviceInfo> & GetDeviceInfos() const { return m_SoundDevices; }

	const SoundDeviceInfo * FindDeviceInfo(SoundDeviceID id) const;

	SoundDeviceCaps GetDeviceCaps(SoundDeviceID id, const std::vector<uint32> &baseSampleRates, ISoundMessageReceiver *messageReceiver = nullptr, ISoundDevice *currentSoundDevice = nullptr);

	ISoundDevice * CreateSoundDevice(SoundDeviceID id);

};
