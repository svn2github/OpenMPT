/*
 * SoundDevicePortAudio.cpp
 * ------------------------
 * Purpose: PortAudio sound device driver class.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "SoundDevice.h"
#include "SoundDevices.h"

#include "SoundDevicePortAudio.h"

#include "../common/misc_util.h"


///////////////////////////////////////////////////////////////////////////////////////
//
// Portaudio Device implementation
//

#ifndef NO_PORTAUDIO

CPortaudioDevice::CPortaudioDevice(SoundDeviceID id, const std::wstring &internalID)
//----------------------------------------------------------------------------------
	: ISoundDevice(id, internalID)
{
	m_HostApi = SndDevTypeToHostApi(id.GetType());
	MemsetZero(m_StreamParameters);
	m_Stream = 0;
	m_CurrentFrameCount = 0;
	m_CurrentRealLatencyMS = 0.0f;
}


CPortaudioDevice::~CPortaudioDevice()
//-----------------------------------
{
	Reset();
	Close();
}


bool CPortaudioDevice::InternalOpen()
//-----------------------------------
{
	MemsetZero(m_StreamParameters);
	m_Stream = 0;
	m_CurrentFrameBuffer = 0;
	m_CurrentFrameCount = 0;
	m_StreamParameters.device = HostApiOutputIndexToGlobalDeviceIndex(GetDeviceIndex(), m_HostApi);
	if(m_StreamParameters.device == -1) return false;
	m_StreamParameters.channelCount = m_Settings.Channels;
	if(m_Settings.sampleFormat.IsFloat())
	{
		if(m_Settings.sampleFormat.GetBitsPerSample() != 32) return false;
		m_StreamParameters.sampleFormat = paFloat32;
	} else
	{
		switch(m_Settings.sampleFormat.GetBitsPerSample())
		{
		case 8: m_StreamParameters.sampleFormat = paUInt8; break;
		case 16: m_StreamParameters.sampleFormat = paInt16; break;
		case 24: m_StreamParameters.sampleFormat = paInt24; break;
		case 32: m_StreamParameters.sampleFormat = paInt32; break;
		default: return false; break;
		}
	}
	m_StreamParameters.suggestedLatency = m_Settings.LatencyMS / 1000.0;
	m_StreamParameters.hostApiSpecificStreamInfo = NULL;
	if((m_HostApi == Pa_HostApiTypeIdToHostApiIndex(paWASAPI)) && m_Settings.ExclusiveMode)
	{
		MemsetZero(m_WasapiStreamInfo);
		m_WasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
		m_WasapiStreamInfo.hostApiType = paWASAPI;
		m_WasapiStreamInfo.version = 1;
		m_WasapiStreamInfo.flags = paWinWasapiExclusive;
		m_StreamParameters.hostApiSpecificStreamInfo = &m_WasapiStreamInfo;
	}
	if(Pa_IsFormatSupported(NULL, &m_StreamParameters, m_Settings.Samplerate) != paFormatIsSupported) return false;
	if(Pa_OpenStream(&m_Stream, NULL, &m_StreamParameters, m_Settings.Samplerate, /*static_cast<long>(m_UpdateIntervalMS * pwfx->nSamplesPerSec / 1000.0f)*/ paFramesPerBufferUnspecified, paNoFlag, StreamCallbackWrapper, (void*)this) != paNoError) return false;
	if(!Pa_GetStreamInfo(m_Stream))
	{
		Pa_CloseStream(m_Stream);
		m_Stream = 0;
		return false;
	}
	m_RealLatencyMS = static_cast<float>(Pa_GetStreamInfo(m_Stream)->outputLatency) * 1000.0f;
	m_RealUpdateIntervalMS = static_cast<float>(m_Settings.UpdateIntervalMS);
	return true;
}


bool CPortaudioDevice::InternalClose()
//------------------------------------
{
	if(m_Stream)
	{
		Pa_AbortStream(m_Stream);
		Pa_CloseStream(m_Stream);
		if(Pa_GetDeviceInfo(m_StreamParameters.device)->hostApi == Pa_HostApiTypeIdToHostApiIndex(paWDMKS)) Pa_Sleep((long)(m_RealLatencyMS*2)); // wait for broken wdm drivers not closing the stream immediatly
		MemsetZero(m_StreamParameters);
		m_Stream = 0;
		m_CurrentFrameCount = 0;
		m_CurrentFrameBuffer = 0;
	}
	return true;
}


void CPortaudioDevice::InternalReset()
//------------------------------------
{
	Pa_AbortStream(m_Stream);
}


void CPortaudioDevice::InternalStart()
//------------------------------------
{
	Pa_StartStream(m_Stream);
}


void CPortaudioDevice::InternalStop()
//-----------------------------------
{
	Pa_StopStream(m_Stream);
}


void CPortaudioDevice::FillAudioBuffer()
//--------------------------------------
{
	if(m_CurrentFrameCount == 0) return;
	SourceAudioRead(m_CurrentFrameBuffer, m_CurrentFrameCount);
	SourceAudioDone(m_CurrentFrameCount, static_cast<ULONG>(m_CurrentRealLatencyMS * Pa_GetStreamInfo(m_Stream)->sampleRate / 1000.0f));
}


int64 CPortaudioDevice::InternalGetStreamPositionSamples() const
//--------------------------------------------------------------
{
	if(!IsOpen()) return 0;
	if(Pa_IsStreamActive(m_Stream) != 1) return 0;
	return static_cast<int64>(Pa_GetStreamTime(m_Stream) * Pa_GetStreamInfo(m_Stream)->sampleRate);
}


float CPortaudioDevice::GetCurrentRealLatencyMS()
//-----------------------------------------------
{
	if(!IsOpen()) return 0.0f;
	return m_CurrentRealLatencyMS;
}


SoundDeviceCaps CPortaudioDevice::GetDeviceCaps(const std::vector<uint32> &baseSampleRates)
//-----------------------------------------------------------------------------------------
{
	SoundDeviceCaps caps;
	PaDeviceIndex device = HostApiOutputIndexToGlobalDeviceIndex(GetDeviceIndex(), m_HostApi);
	if(device == -1)
	{
		return caps;
	}
	for(UINT n=0; n<baseSampleRates.size(); n++)
	{
		PaStreamParameters StreamParameters;
		MemsetZero(StreamParameters);
		StreamParameters.device = device;
		StreamParameters.channelCount = 2;
		StreamParameters.sampleFormat = paInt16;
		StreamParameters.suggestedLatency = 0.0;
		StreamParameters.hostApiSpecificStreamInfo = NULL;
		if((m_HostApi == Pa_HostApiTypeIdToHostApiIndex(paWASAPI)) && m_Settings.ExclusiveMode)
		{
			MemsetZero(m_WasapiStreamInfo);
			m_WasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
			m_WasapiStreamInfo.hostApiType = paWASAPI;
			m_WasapiStreamInfo.version = 1;
			m_WasapiStreamInfo.flags = paWinWasapiExclusive;
			m_StreamParameters.hostApiSpecificStreamInfo = &m_WasapiStreamInfo;
		}
		if(Pa_IsFormatSupported(NULL, &StreamParameters, baseSampleRates[n]) == paFormatIsSupported)
		{
			caps.supportedSampleRates.push_back(baseSampleRates[n]);
		}
	}
	return caps;
}


int CPortaudioDevice::StreamCallback(
	const void *input, void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags
	)
//-----------------------------------------
{
	MPT_UNREFERENCED_PARAMETER(input);
	MPT_UNREFERENCED_PARAMETER(statusFlags);
	if(!output) return paAbort;
	if(Pa_GetHostApiInfo(m_HostApi)->type == paWDMKS)
	{
		// For WDM-KS, timeInfo->outputBufferDacTime seems to contain bogus values.
		// Work-around it by using the slightly less accurate per-stream latency estimation.
		m_CurrentRealLatencyMS = static_cast<float>( Pa_GetStreamInfo(m_Stream)->outputLatency * 1000.0 );
	} else
	{
		m_CurrentRealLatencyMS = static_cast<float>( timeInfo->outputBufferDacTime - timeInfo->currentTime ) * 1000.0f;
	}
	m_CurrentFrameBuffer = output;
	m_CurrentFrameCount = frameCount;
	SourceFillAudioBufferLocked();
	m_CurrentFrameCount = 0;
	m_CurrentFrameBuffer = 0;
	return paContinue;
}


int CPortaudioDevice::StreamCallbackWrapper(
	const void *input, void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData
	)
//------------------------------------------
{
	return ((CPortaudioDevice*)userData)->StreamCallback(input, output, frameCount, timeInfo, statusFlags);
}


PaDeviceIndex CPortaudioDevice::HostApiOutputIndexToGlobalDeviceIndex(int hostapioutputdeviceindex, PaHostApiIndex hostapi)
//-------------------------------------------------------------------------------------------------------------------
{
	if(hostapi < 0)
		return -1;
	if(hostapi >= Pa_GetHostApiCount())
		return -1;
	if(!Pa_GetHostApiInfo(hostapi))
		return -1;
	if(hostapioutputdeviceindex < 0)
		return -1;
	if(hostapioutputdeviceindex >= Pa_GetHostApiInfo(hostapi)->deviceCount)
		return -1;
	int dev = hostapioutputdeviceindex;
	for(int hostapideviceindex=0; hostapideviceindex<Pa_GetHostApiInfo(hostapi)->deviceCount; hostapideviceindex++)
	{
		if(!Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostapi, hostapideviceindex)))
		{
			dev++; // skip this device
			continue;
		}
		if(Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostapi, hostapideviceindex))->maxOutputChannels == 0)
		{
			dev++; // skip this device
			continue;
		}
		if(dev == hostapideviceindex)
		{
			break;
		}
	}
	if(dev >= Pa_GetHostApiInfo(hostapi)->deviceCount)
		return -1;
	return Pa_HostApiDeviceIndexToDeviceIndex(hostapi, dev);
}


SoundDeviceType CPortaudioDevice::HostApiToSndDevType(PaHostApiIndex hostapi)
//---------------------------------------------------------------------------
{
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paWASAPI)) return SNDDEV_PORTAUDIO_WASAPI;
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paWDMKS)) return SNDDEV_PORTAUDIO_WDMKS;
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paMME)) return SNDDEV_PORTAUDIO_WMME;
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paDirectSound)) return SNDDEV_PORTAUDIO_DS;
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paASIO)) return SNDDEV_PORTAUDIO_ASIO;
	return SNDDEV_INVALID;
}


PaHostApiIndex CPortaudioDevice::SndDevTypeToHostApi(SoundDeviceType snddevtype)
//------------------------------------------------------------------------------
{
	if(snddevtype == SNDDEV_PORTAUDIO_WASAPI) return Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
	if(snddevtype == SNDDEV_PORTAUDIO_WDMKS) return Pa_HostApiTypeIdToHostApiIndex(paWDMKS);
	if(snddevtype == SNDDEV_PORTAUDIO_WMME) return Pa_HostApiTypeIdToHostApiIndex(paMME);
	if(snddevtype == SNDDEV_PORTAUDIO_DS) return Pa_HostApiTypeIdToHostApiIndex(paDirectSound);
	if(snddevtype == SNDDEV_PORTAUDIO_ASIO) return Pa_HostApiTypeIdToHostApiIndex(paASIO);
	return paInDevelopment;
}


std::string CPortaudioDevice::HostApiToString(PaHostApiIndex hostapi)
//-------------------------------------------------------------------
{
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paWASAPI)) return "WASAPI";
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paWDMKS)) return "WDM-KS";
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paMME)) return "MME";
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paDirectSound)) return "DS";
	if(hostapi == Pa_HostApiTypeIdToHostApiIndex(paASIO)) return "ASIO";
	return "PortAudio";
}


bool CPortaudioDevice::EnumerateDevices(SoundDeviceInfo &result, SoundDeviceIndex index, PaHostApiIndex hostapi)
//--------------------------------------------------------------------------------------------------------------
{
	result = SoundDeviceInfo();
	PaDeviceIndex dev = HostApiOutputIndexToGlobalDeviceIndex(index, hostapi);
	if(dev == -1)
		return false;
	if(!Pa_GetDeviceInfo(dev))
		return false;
	result.id = SoundDeviceID(HostApiToSndDevType(hostapi), index);
	result.name = mpt::String::Decode(
		mpt::String::Format("%s%s",
			Pa_GetDeviceInfo(dev)->name,
			Pa_GetHostApiInfo(Pa_GetDeviceInfo(dev)->hostApi)->defaultOutputDevice == (PaDeviceIndex)dev ? " (Default)" : ""
		),
		mpt::CharsetUTF8);
	result.apiName = mpt::String::Decode(HostApiToString(Pa_GetDeviceInfo(dev)->hostApi).c_str(), mpt::CharsetUTF8);
	return true;
}


std::vector<SoundDeviceInfo> CPortaudioDevice::EnumerateDevices(SoundDeviceType type)
//-----------------------------------------------------------------------------------
{
	std::vector<SoundDeviceInfo> devices;
	if(!SndDevPortaudioIsInitialized())
	{
		return devices;
	}
	for(SoundDeviceIndex index = 0; ; ++index)
	{
		SoundDeviceInfo info;
		if(!EnumerateDevices(info, index, CPortaudioDevice::SndDevTypeToHostApi(type)))
		{
			break;
		}
		devices.push_back(info);
	}
	return devices;
}


static bool g_PortaudioInitialized = false;


void SndDevPortaudioInitialize()
//------------------------------
{
	if(Pa_Initialize() != paNoError) return;
	g_PortaudioInitialized = true;
}


void SndDevPortaudioUnnitialize()
//-------------------------------
{
	if(!g_PortaudioInitialized) return;
	Pa_Terminate();
	g_PortaudioInitialized = false;
}


bool SndDevPortaudioIsInitialized()
//---------------------------------
{
	return g_PortaudioInitialized;
}


#endif // NO_PORTAUDIO
