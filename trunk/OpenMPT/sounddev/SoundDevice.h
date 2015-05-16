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
#include "../common/misc_util.h"
#include "../common/FlagSet.h"
#include "../common/mptAtomic.h"
#include "../soundlib/SampleFormat.h"

#include <vector>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice {


//====================
class IMessageReceiver
//====================
{
public:
	virtual void SoundDeviceMessage(LogLevel level, const mpt::ustring &str) = 0;
};


struct StreamPosition
{
	int64 Frames; // relative to Start()
	double Seconds; // relative to Start()
	StreamPosition() : Frames(0), Seconds(0.0) { }
	StreamPosition(int64 frames, double seconds) : Frames(frames), Seconds(seconds) { }
};


struct TimeInfo
{
	
	int64 SyncPointStreamFrames;
	uint64 SyncPointSystemTimestamp;
	double Speed;

	SoundDevice::StreamPosition RenderStreamPositionBefore;
	SoundDevice::StreamPosition RenderStreamPositionAfter;
	// int64 chunkSize = After - Before
	
	TimeInfo()
		: SyncPointStreamFrames(0)
		, SyncPointSystemTimestamp(0)
		, Speed(1.0)
	{
		return;
	}

};


struct Settings;
struct Flags;
struct BufferAttributes;


//===========
class ISource
//===========
{
public:
	virtual uint64 SoundSourceGetReferenceClockNowNanoseconds() const = 0; // timeGetTime()*1000000 on Windows
	virtual void SoundSourcePreStartCallback() = 0;
	virtual void SoundSourcePostStopCallback() = 0;
	virtual bool SoundSourceIsLockedByCurrentThread() const = 0;
	virtual void SoundSourceLock() = 0;
	virtual void SoundSourceRead(const SoundDevice::Settings &settings, const SoundDevice::Flags &flags, const SoundDevice::BufferAttributes &bufferAttributes, SoundDevice::TimeInfo timeInfo, std::size_t numFrames, void *buffer) = 0;
	virtual void SoundSourceDone(const SoundDevice::Settings &settings, const SoundDevice::Flags &flags, const SoundDevice::BufferAttributes &bufferAttributes, SoundDevice::TimeInfo timeInfo) = 0;
	virtual void SoundSourceUnlock() = 0;
public:
	class Guard
	{
	private:
		ISource &m_Source;
	public:
		Guard(ISource &source)
			: m_Source(source)
		{
			m_Source.SoundSourceLock();
		}
		~Guard()
		{
			m_Source.SoundSourceUnlock();
		}
	};
};


static const MPT_UCHAR_TYPE TypeWAVEOUT          [] = MPT_ULITERAL("WaveOut");
static const MPT_UCHAR_TYPE TypeDSOUND           [] = MPT_ULITERAL("DirectSound");
static const MPT_UCHAR_TYPE TypeASIO             [] = MPT_ULITERAL("ASIO");
static const MPT_UCHAR_TYPE TypePORTAUDIO_WASAPI [] = MPT_ULITERAL("WASAPI");
static const MPT_UCHAR_TYPE TypePORTAUDIO_WDMKS  [] = MPT_ULITERAL("WDM-KS");
static const MPT_UCHAR_TYPE TypePORTAUDIO_WMME   [] = MPT_ULITERAL("MME");
static const MPT_UCHAR_TYPE TypePORTAUDIO_DS     [] = MPT_ULITERAL("DS");

typedef mpt::ustring Type;


typedef mpt::ustring Identifier;

SoundDevice::Type ParseType(const SoundDevice::Identifier &identifier);

struct Info
{
	SoundDevice::Type type;
	mpt::ustring internalID;
	mpt::ustring name; // user visible
	mpt::ustring apiName; // user visible
	std::vector<mpt::ustring> apiPath; // i.e. Wine-support, PortAudio
	bool isDefault;
	bool useNameAsIdentifier;
	Info() : isDefault(false), useNameAsIdentifier(false) { }
	bool IsValid() const
	{
		return !type.empty() && !internalID.empty();
	}
	SoundDevice::Identifier GetIdentifier() const
	{
		if(!IsValid())
		{
			return mpt::ustring();
		}
		mpt::ustring result = mpt::ustring();
		result += type;
		result += MPT_USTRING("_");
		if(useNameAsIdentifier)
		{
			// UTF8-encode the name and convert the utf8 to hex.
			// This ensures that no special characters are contained in the configuration key.
			std::string utf8String = mpt::ToCharset(mpt::CharsetUTF8, name);
			mpt::ustring hexString = Util::BinToHex(std::vector<char>(utf8String.begin(), utf8String.end()));
			result += hexString;
		} else
		{
			result += internalID; // safe to not contain special characters
		}
		return result;
	}
};


struct ChannelMapping
{

private:

	std::vector<int32> ChannelToDeviceChannel;

public:

	static const int32 MaxDeviceChannel = 32000;

public:

	// Construct default empty mapping
	ChannelMapping();

	// Construct default identity mapping
	ChannelMapping(uint32 numHostChannels);

	// Construct mapping from given vector.
	// Silently fall back to identity mapping if mapping is invalid.
	ChannelMapping(const std::vector<int32> &mapping);

	// Construct mapping for #channels with a baseChannel offset.
	static ChannelMapping BaseChannel(uint32 channels, int32 baseChannel);

private:

	// check that the channel mapping is actually a 1:1 mapping
	static bool IsValid(const std::vector<int32> &mapping);

public:

	bool operator == (const SoundDevice::ChannelMapping &cmp) const
	{
		return (ChannelToDeviceChannel == cmp.ChannelToDeviceChannel);
	}
	
	uint32 GetNumHostChannels() const
	{
		return static_cast<uint32>(ChannelToDeviceChannel.size());
	}
	
	// Get the number of required device channels for this mapping. Derived from the maximum mapped-to channel number.
	int32 GetRequiredDeviceChannels() const
	{
		if(ChannelToDeviceChannel.empty())
		{
			return 0;
		}
		int32 maxChannel = 0;
		for(uint32 channel = 0; channel < ChannelToDeviceChannel.size(); ++channel)
		{
			if(ChannelToDeviceChannel[channel] > maxChannel)
			{
				maxChannel = ChannelToDeviceChannel[channel];
			}
		}
		return maxChannel + 1;
	}
	
	// Convert OpenMPT channel number to the mapped device channel number.
	int32 ToDevice(uint32 channel) const
	{
		if(channel >= ChannelToDeviceChannel.size())
		{
			return channel;
		}
		return ChannelToDeviceChannel[channel];
	}

	mpt::ustring ToString() const;

	static SoundDevice::ChannelMapping FromString(const mpt::ustring &str);

};


struct AppInfo
{
	mpt::ustring Name;
	uintptr_t UIHandle; // HWND on Windows
	AppInfo()
		: UIHandle(0)
	{
		return;
	}
	AppInfo &SetName(const mpt::ustring &name) { Name = name; return *this; }
	mpt::ustring GetName() const { return Name; }
#if MPT_OS_WINDOWS
	AppInfo &SetHWND(HWND hwnd) { UIHandle = reinterpret_cast<uintptr_t>(hwnd); return *this; }
	HWND GetHWND() const { return reinterpret_cast<HWND>(UIHandle); }
#endif // MPT_OS_WINDOWS
};


struct Settings
{
	double Latency; // seconds
	double UpdateInterval; // seconds
	uint32 Samplerate;
	uint8 Channels;
	SampleFormat sampleFormat;
	bool ExclusiveMode; // Use hardware buffers directly
	bool BoostThreadPriority; // Boost thread priority for glitch-free audio rendering
	bool KeepDeviceRunning;
	bool UseHardwareTiming;
	int DitherType;
	SoundDevice::ChannelMapping ChannelMapping;
	Settings()
		: Latency(0.1)
		, UpdateInterval(0.005)
		, Samplerate(48000)
		, Channels(2)
		, sampleFormat(SampleFormatFloat32)
		, ExclusiveMode(false)
		, BoostThreadPriority(true)
		, KeepDeviceRunning(true)
		, UseHardwareTiming(false)
		, DitherType(1)
	{
		return;
	}
	bool operator == (const SoundDevice::Settings &cmp) const
	{
		return true
			&& Util::Round<int64>(Latency * 1000000000.0) == Util::Round<int64>(cmp.Latency * 1000000000.0) // compare in nanoseconds
			&& Util::Round<int64>(UpdateInterval * 1000000000.0) == Util::Round<int64>(cmp.UpdateInterval * 1000000000.0) // compare in nanoseconds
			&& Samplerate == cmp.Samplerate
			&& Channels == cmp.Channels
			&& sampleFormat == cmp.sampleFormat
			&& ExclusiveMode == cmp.ExclusiveMode
			&& BoostThreadPriority == cmp.BoostThreadPriority
			&& KeepDeviceRunning == cmp.KeepDeviceRunning
			&& UseHardwareTiming == cmp.UseHardwareTiming
			&& ChannelMapping == cmp.ChannelMapping
			&& DitherType == cmp.DitherType
			;
	}
	bool operator != (const SoundDevice::Settings &cmp) const
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


struct Flags
{
	// Windows since Vista has a limiter/compressor in the audio path that kicks
	// in as soon as there are samples > 0dBFs (i.e. the absolute float value >
	// 1.0). This happens for all APIs that get processed through the system-
	// wide audio engine. It does not happen for exclusive mode WASAPI streams
	// or direct WaveRT (labeled WDM-KS in PortAudio) streams. As there is no
	// known way to disable this annoying behavior, avoid unclipped samples on
	// affected windows versions and clip them ourselves before handing them to
	// the APIs.
	bool NeedsClippedFloat;
	Flags()
		: NeedsClippedFloat(false)
	{
		return;
	}
};


struct Caps
{
	bool Available;
	bool CanUpdateInterval;
	bool CanSampleFormat;
	bool CanExclusiveMode;
	bool CanBoostThreadPriority;
	bool CanKeepDeviceRunning;
	bool CanUseHardwareTiming;
	bool CanChannelMapping;
	bool CanDriverPanel;
	bool HasInternalDither;
	mpt::ustring ExclusiveModeDescription;
	double LatencyMin;
	double LatencyMax;
	double UpdateIntervalMin;
	double UpdateIntervalMax;
	SoundDevice::Settings DefaultSettings;
	Caps()
		: Available(false)
		, CanUpdateInterval(true)
		, CanSampleFormat(true)
		, CanExclusiveMode(false)
		, CanBoostThreadPriority(true)
		, CanKeepDeviceRunning(false)
		, CanUseHardwareTiming(false)
		, CanChannelMapping(false)
		, CanDriverPanel(false)
		, HasInternalDither(false)
		, ExclusiveModeDescription(MPT_USTRING("Use device exclusively"))
		, LatencyMin(0.002) // 2ms
		, LatencyMax(0.5) // 500ms
		, UpdateIntervalMin(0.001) // 1ms
		, UpdateIntervalMax(0.2) // 200ms
	{
		return;
	}
};


struct DynamicCaps
{
	uint32 currentSampleRate;
	std::vector<uint32> supportedSampleRates;
	std::vector<uint32> supportedExclusiveSampleRates;
	std::vector<mpt::ustring> channelNames;
	DynamicCaps()
		: currentSampleRate(0)
	{
		return;
	}
};


struct BufferAttributes
{
	double Latency; // seconds
	double UpdateInterval; // seconds
	int NumBuffers;
	BufferAttributes()
		: Latency(0.0)
		, UpdateInterval(0.0)
		, NumBuffers(0)
	{
		return;
	}
};


enum RequestFlags
{
	RequestFlagClose   = 1<<0,
	RequestFlagReset   = 1<<1,
	RequestFlagRestart = 1<<2,
};
} // namespace SoundDevice
template <> struct enum_traits<SoundDevice::RequestFlags> { typedef uint32 store_type; };
namespace SoundDevice {
MPT_DECLARE_ENUM(RequestFlags)


struct Statistics
{
	double InstantaneousLatency;
	double LastUpdateInterval;
	mpt::ustring text;
	Statistics()
		: InstantaneousLatency(0.0)
		, LastUpdateInterval(0.0)
	{
		return;
	}
};


//=========
class IBase
//=========
{

protected:

	IBase() { }

public:

	virtual ~IBase() { }

public:

	virtual void SetSource(SoundDevice::ISource *source) = 0;
	virtual void SetMessageReceiver(SoundDevice::IMessageReceiver *receiver) = 0;

	virtual SoundDevice::Info GetDeviceInfo() const = 0;

	virtual SoundDevice::Caps GetDeviceCaps() const = 0;
	virtual SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates) = 0;

	virtual bool Init(const SoundDevice::AppInfo &appInfo) = 0;
	virtual bool Open(const SoundDevice::Settings &settings) = 0;
	virtual bool Close() = 0;
	virtual bool Start() = 0;
	virtual void Stop(bool force = false) = 0;

	virtual FlagSet<RequestFlags> GetRequestFlags() const = 0;

	virtual bool IsInited() const = 0;
	virtual bool IsOpen() const = 0;
	virtual bool IsAvailable() const = 0;
	virtual bool IsPlaying() const = 0;

	virtual bool OnIdle() = 0; // return true if any work has been done

	virtual SoundDevice::Settings GetSettings() const = 0;
	virtual SampleFormat GetActualSampleFormat() const = 0;
	virtual SoundDevice::BufferAttributes GetEffectiveBufferAttributes() const = 0;

	virtual SoundDevice::TimeInfo GetTimeInfo() const = 0;
	virtual SoundDevice::StreamPosition GetStreamPosition() const = 0;

	// Debugging aids in case of a crash
	virtual bool DebugIsFragileDevice() const = 0;
	virtual bool DebugInRealtimeCallback() const = 0;

	// Informational only, do not use for timing.
	// Use GetStreamPositionFrames() for timing
	virtual SoundDevice::Statistics GetStatistics() const = 0;

	virtual bool OpenDriverSettings() = 0;

};


//========
class Base
//========
	: public IBase
{

private:

	SoundDevice::ISource *m_Source;
	SoundDevice::IMessageReceiver *m_MessageReceiver;

	const SoundDevice::Info m_Info;

private:

	SoundDevice::Caps m_Caps;

protected:

	SoundDevice::AppInfo m_AppInfo;
	SoundDevice::Settings m_Settings;
	SoundDevice::Flags m_Flags;
	bool m_DeviceUnavailableOnOpen;

private:

	bool m_IsPlaying;

	SoundDevice::TimeInfo m_TimeInfo;

	int64 m_StreamPositionRenderFrames; // only updated or read in audio CALLBACK or when device is stopped. requires no further locking

	mutable Util::mutex m_StreamPositionMutex;
	int64 m_StreamPositionOutputFrames;

	mpt::atomic_uint32_t m_RequestFlags;

protected:

	SoundDevice::Type GetDeviceType() const { return m_Info.type; }
	mpt::ustring GetDeviceInternalID() const { return m_Info.internalID; }
	SoundDevice::Identifier GetDeviceIdentifier() const { return m_Info.GetIdentifier(); }

	virtual void InternalFillAudioBuffer() = 0;

	uint64 SourceGetReferenceClockNowNanoseconds() const;
	void SourceNotifyPreStart();
	void SourceNotifyPostStop();
	bool SourceIsLockedByCurrentThread() const;
	void SourceFillAudioBufferLocked();
	void SourceAudioPreRead(std::size_t numFrames, std::size_t framesLatency);
	void SourceAudioRead(void *buffer, std::size_t numFrames);
	void SourceAudioDone();

	void RequestClose() { m_RequestFlags.fetch_or(RequestFlagClose); }
	void RequestReset() { m_RequestFlags.fetch_or(RequestFlagReset); }
	void RequestRestart() { m_RequestFlags.fetch_or(RequestFlagRestart); }

	void SendDeviceMessage(LogLevel level, const mpt::ustring &str);

protected:

	void SetTimeInfo(SoundDevice::TimeInfo timeInfo) { m_TimeInfo = timeInfo; }

	SoundDevice::StreamPosition StreamPositionFromFrames(int64 frames) const { return SoundDevice::StreamPosition(frames, static_cast<double>(frames) / static_cast<double>(m_Settings.Samplerate)); }

	virtual bool InternalHasTimeInfo() const { return false; }

	virtual bool InternalHasGetStreamPosition() const { return false; }
	virtual int64 InternalGetStreamPositionFrames() const { return 0; }

	virtual bool InternalIsOpen() const = 0;

	virtual bool InternalOpen() = 0;
	virtual bool InternalStart() = 0;
	virtual void InternalStop() = 0;
	virtual void InternalStopForce() { InternalStop(); }
	virtual bool InternalClose() = 0;

	virtual SoundDevice::Caps InternalGetDeviceCaps() = 0;

	virtual SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const = 0;

protected:

	Base(SoundDevice::Info info);

public:

	virtual ~Base();

	void SetSource(SoundDevice::ISource *source) { m_Source = source; }
	void SetMessageReceiver(SoundDevice::IMessageReceiver *receiver) { m_MessageReceiver = receiver; }

	SoundDevice::Info GetDeviceInfo() const { return m_Info; }

	SoundDevice::Caps GetDeviceCaps() const { return m_Caps; }
	virtual SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);

	bool Init(const SoundDevice::AppInfo &appInfo);
	bool Open(const SoundDevice::Settings &settings);
	bool Close();
	bool Start();
	void Stop(bool force = false);

	FlagSet<RequestFlags> GetRequestFlags() const { return FlagSet<RequestFlags>(m_RequestFlags.load()); }

	bool IsInited() const { return m_Caps.Available; }
	bool IsOpen() const { return IsInited() && InternalIsOpen(); }
	bool IsAvailable() const { return m_Caps.Available && !m_DeviceUnavailableOnOpen; }
	bool IsPlaying() const { return m_IsPlaying; }

	virtual bool OnIdle() { return false; }

	SoundDevice::Settings GetSettings() const { return m_Settings; }
	SampleFormat GetActualSampleFormat() const { return IsOpen() ? m_Settings.sampleFormat : SampleFormatInvalid; }
	SoundDevice::BufferAttributes GetEffectiveBufferAttributes() const { return (IsOpen() && IsPlaying()) ? InternalGetEffectiveBufferAttributes() : SoundDevice::BufferAttributes(); }

	SoundDevice::TimeInfo GetTimeInfo() const { return m_TimeInfo; }
	SoundDevice::StreamPosition GetStreamPosition() const;

	virtual bool DebugIsFragileDevice() const { return false; }
	virtual bool DebugInRealtimeCallback() const { return false; }

	virtual SoundDevice::Statistics GetStatistics() const;

	virtual bool OpenDriverSettings() { return false; };

};


namespace Legacy
{
typedef uint16 ID;
static const SoundDevice::Legacy::ID MaskType = 0xff00;
static const SoundDevice::Legacy::ID MaskIndex = 0x00ff;
static const SoundDevice::Legacy::ID TypeWAVEOUT          = 0;
static const SoundDevice::Legacy::ID TypeDSOUND           = 1;
static const SoundDevice::Legacy::ID TypeASIO             = 2;
static const SoundDevice::Legacy::ID TypePORTAUDIO_WASAPI = 3;
static const SoundDevice::Legacy::ID TypePORTAUDIO_WDMKS  = 4;
static const SoundDevice::Legacy::ID TypePORTAUDIO_WMME   = 5;
static const SoundDevice::Legacy::ID TypePORTAUDIO_DS     = 6;
mpt::ustring GetDirectSoundDefaultDeviceIdentifierPre_1_25_00_04();
mpt::ustring GetDirectSoundDefaultDeviceIdentifier_1_25_00_04();
}


} // namespace SoundDevice


OPENMPT_NAMESPACE_END
