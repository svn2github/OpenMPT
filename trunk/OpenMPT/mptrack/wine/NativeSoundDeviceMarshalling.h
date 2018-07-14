
#pragma once

#include "NativeSoundDevice.h"

#include "../../sounddev/SoundDevice.h"

#ifdef MPT_WITH_NLOHMANNJSON

#include "../../misc/JSON.h"

#endif // MPT_WITH_NLOHMANNJSON



OPENMPT_NAMESPACE_BEGIN



#ifdef MPT_WITH_NLOHMANNJSON

inline void to_json(JSON::value &j, const SampleFormat &val)
{
	j = static_cast<int>(val);
}
inline void from_json(const JSON::value &j, SampleFormat &val)
{
	val = static_cast<int>(j);
}

namespace SoundDevice
{

	inline void to_json(JSON::value &j, const SoundDevice::ChannelMapping &val)
	{
		j = val.ToUString();
	}
	inline void from_json(const JSON::value &j, SoundDevice::ChannelMapping &val)
	{
		val = SoundDevice::ChannelMapping::FromString(j);
	}

} // namespace SoundDevice



namespace SoundDevice
{

	MPT_JSON_INLINE(SoundDevice::Info, {
		MPT_JSON_MAP(type);
		MPT_JSON_MAP(internalID);
		MPT_JSON_MAP(name);
		MPT_JSON_MAP(apiName);
		MPT_JSON_MAP(apiPath);
		MPT_JSON_MAP(isDefault);
		MPT_JSON_MAP(useNameAsIdentifier);
		MPT_JSON_MAP(extraData);
	})

	MPT_JSON_INLINE(SoundDevice::AppInfo, {
		MPT_JSON_MAP(Name);
		MPT_JSON_MAP(BoostedThreadPriorityXP);
		MPT_JSON_MAP(BoostedThreadMMCSSClassVista);
		MPT_JSON_MAP(BoostedThreadRealtimePosix);
		MPT_JSON_MAP(BoostedThreadNicenessPosix);
		MPT_JSON_MAP(BoostedThreadRtprioPosix);
	})

	MPT_JSON_INLINE(SoundDevice::Settings, {
		MPT_JSON_MAP(Latency);
		MPT_JSON_MAP(UpdateInterval);
		MPT_JSON_MAP(Samplerate);
		MPT_JSON_MAP(Channels);
		MPT_JSON_MAP(InputChannels);
		MPT_JSON_MAP(sampleFormat);
		MPT_JSON_MAP(ExclusiveMode);
		MPT_JSON_MAP(BoostThreadPriority);
		MPT_JSON_MAP(KeepDeviceRunning);
		MPT_JSON_MAP(UseHardwareTiming);
		MPT_JSON_MAP(DitherType);
		MPT_JSON_MAP(InputSourceID);
	})

	MPT_JSON_INLINE(SoundDevice::Caps, {
		MPT_JSON_MAP(Available);
		MPT_JSON_MAP(CanUpdateInterval);
		MPT_JSON_MAP(CanSampleFormat);
		MPT_JSON_MAP(CanExclusiveMode);
		MPT_JSON_MAP(CanBoostThreadPriority);
		MPT_JSON_MAP(CanKeepDeviceRunning);
		MPT_JSON_MAP(CanUseHardwareTiming);
		MPT_JSON_MAP(CanChannelMapping);
		MPT_JSON_MAP(CanInput);
		MPT_JSON_MAP(HasNamedInputSources);
		MPT_JSON_MAP(CanDriverPanel);
		MPT_JSON_MAP(HasInternalDither);
		MPT_JSON_MAP(ExclusiveModeDescription);
		MPT_JSON_MAP(LatencyMin);
		MPT_JSON_MAP(LatencyMax);
		MPT_JSON_MAP(UpdateIntervalMin);
		MPT_JSON_MAP(UpdateIntervalMax);
		MPT_JSON_MAP(DefaultSettings);
	})

	MPT_JSON_INLINE(SoundDevice::DynamicCaps, {
		MPT_JSON_MAP(currentSampleRate);
		MPT_JSON_MAP(supportedSampleRates);
		MPT_JSON_MAP(supportedExclusiveSampleRates);
		MPT_JSON_MAP(channelNames);
		MPT_JSON_MAP(inputSourceNames);
	})

	MPT_JSON_INLINE(SoundDevice::Statistics, {
		MPT_JSON_MAP(InstantaneousLatency);
		MPT_JSON_MAP(LastUpdateInterval);
		MPT_JSON_MAP(text);
	})

} // namespace SoundDevice



template <typename Tdst, typename Tsrc>
struct json_cast_impl
{
	Tdst operator () (const Tsrc &src);
};


template <typename Tdst, typename Tsrc>
Tdst json_cast(const Tsrc &src)
{
	return json_cast_impl<Tdst, Tsrc>()(src);
}


template <typename Tsrc>
struct json_cast_impl<JSON::value, Tsrc>
{
	JSON::value operator () (const Tsrc &src)
	{
		JSON::value j;
		JSON::enc(j, src);
		return j;
	}
};

template <typename Tdst>
struct json_cast_impl<Tdst, JSON::value>
{
	Tdst operator () (const JSON::value &j)
	{
		Tdst val;
		JSON::dec(val, j);
		return val;
	}
};

template <typename Tsrc>
struct json_cast_impl<std::string, Tsrc>
{
	std::string operator () (const Tsrc &src)
	{
		return JSON::serialize(json_cast<JSON::value>(src));
	}
};

template <typename Tdst>
struct json_cast_impl<Tdst, std::string>
{
	Tdst operator () (const std::string &str)
	{
		return json_cast<Tdst>(JSON::deserialize(str));
	}
};

#endif // MPT_WITH_NLOHMANNJSON



namespace C {

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_StreamPosition) % 8 == 0);
inline OpenMPT_SoundDevice_StreamPosition encode(SoundDevice::StreamPosition src) {
	OpenMPT_SoundDevice_StreamPosition dst;
	MemsetZero(dst);
	dst.Frames = src.Frames;
	dst.Seconds = src.Seconds;
	return dst;
}
inline SoundDevice::StreamPosition decode(OpenMPT_SoundDevice_StreamPosition src) {
	SoundDevice::StreamPosition dst;
	dst.Frames = src.Frames;
	dst.Seconds = src.Seconds;
	return dst;
}

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_TimeInfo) % 8 == 0);
inline OpenMPT_SoundDevice_TimeInfo encode(SoundDevice::TimeInfo src) {
	OpenMPT_SoundDevice_TimeInfo dst;
	MemsetZero(dst);
	dst.SyncPointStreamFrames = src.SyncPointStreamFrames;
	dst.SyncPointSystemTimestamp = src.SyncPointSystemTimestamp;
	dst.Speed = src.Speed;
	dst.RenderStreamPositionBefore = encode(src.RenderStreamPositionBefore);
	dst.RenderStreamPositionAfter = encode(src.RenderStreamPositionAfter);
	return dst;
}
inline SoundDevice::TimeInfo decode(OpenMPT_SoundDevice_TimeInfo src) {
	SoundDevice::TimeInfo dst;
	dst.SyncPointStreamFrames = src.SyncPointStreamFrames;
	dst.SyncPointSystemTimestamp = src.SyncPointSystemTimestamp;
	dst.Speed = src.Speed;
	dst.RenderStreamPositionBefore = decode(src.RenderStreamPositionBefore);
	dst.RenderStreamPositionAfter = decode(src.RenderStreamPositionAfter);
	return dst;
}

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_Flags) % 8 == 0);
inline OpenMPT_SoundDevice_Flags encode(SoundDevice::Flags src) {
	OpenMPT_SoundDevice_Flags dst;
	MemsetZero(dst);
	dst.NeedsClippedFloat = src.NeedsClippedFloat;
	return dst;
}
inline SoundDevice::Flags decode(OpenMPT_SoundDevice_Flags src) {
	SoundDevice::Flags dst;
	dst.NeedsClippedFloat = src.NeedsClippedFloat;
	return dst;
}

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_BufferFormat) % 8 == 0);
inline OpenMPT_SoundDevice_BufferFormat encode(SoundDevice::BufferFormat src) {
	OpenMPT_SoundDevice_BufferFormat dst;
	MemsetZero(dst);
	dst.Samplerate = src.Samplerate;
	dst.Channels = src.Channels;
	dst.InputChannels = src.InputChannels;
	dst.sampleFormat = src.sampleFormat;
	dst.NeedsClippedFloat = src.NeedsClippedFloat;
	dst.DitherType = src.DitherType;
	return dst;
}
inline SoundDevice::BufferFormat decode(OpenMPT_SoundDevice_BufferFormat src) {
	SoundDevice::BufferFormat dst;
	dst.Samplerate = src.Samplerate;
	dst.Channels = src.Channels;
	dst.InputChannels = src.InputChannels;
	dst.sampleFormat = src.sampleFormat;
	dst.NeedsClippedFloat = src.NeedsClippedFloat;
	dst.DitherType = src.DitherType;
	return dst;
}

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_BufferAttributes) % 8 == 0);
inline OpenMPT_SoundDevice_BufferAttributes encode(SoundDevice::BufferAttributes src) {
	OpenMPT_SoundDevice_BufferAttributes dst;
	MemsetZero(dst);
	dst.Latency = src.Latency;
	dst.UpdateInterval = src.UpdateInterval;
	dst.NumBuffers = src.NumBuffers;
	return dst;
}
inline SoundDevice::BufferAttributes decode(OpenMPT_SoundDevice_BufferAttributes src) {
	SoundDevice::BufferAttributes dst;
	dst.Latency = src.Latency;
	dst.UpdateInterval = src.UpdateInterval;
	dst.NumBuffers = src.NumBuffers;
	return dst;
}

STATIC_ASSERT(sizeof(OpenMPT_SoundDevice_RequestFlags) % 8 == 0);
inline OpenMPT_SoundDevice_RequestFlags encode(FlagSet<SoundDevice::RequestFlags> src) {
	OpenMPT_SoundDevice_RequestFlags dst;
	MemsetZero(dst);
	dst.RequestFlags = src.GetRaw();
	return dst;
}
inline FlagSet<SoundDevice::RequestFlags> decode(OpenMPT_SoundDevice_RequestFlags src) {
	FlagSet<SoundDevice::RequestFlags> dst;
	dst.SetRaw(src.RequestFlags);
	return dst;
}

} // namespace C



OPENMPT_NAMESPACE_END
