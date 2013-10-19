/*
 * SoundDeviceASIO.h
 * -----------------
 * Purpose: ASIO sound device driver class.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "SoundDevices.h"

#ifndef NO_ASIO
#include <iasiodrv.h>
#define ASIO_LOG
#endif

////////////////////////////////////////////////////////////////////////////////////
//
// ASIO device
//

#ifndef NO_ASIO

//====================================
class CASIODevice: public ISoundDevice
//====================================
{
	enum { ASIO_MAX_CHANNELS=4 };
	enum { ASIO_BLOCK_LEN=1024 };
protected:
	IASIO *m_pAsioDrv;
	UINT m_nAsioBufferLen;
	UINT m_nAsioSampleSize;
	bool m_Float;
	BOOL m_bMixRunning;
	BOOL m_bPostOutput;
	LONG m_RenderSilence;
	LONG m_RenderingSilence;
	ASIOCallbacks m_Callbacks;
	ASIOChannelInfo m_ChannelInfo[ASIO_MAX_CHANNELS];
	ASIOBufferInfo m_BufferInfo[ASIO_MAX_CHANNELS];
	int32 m_FrameBuffer[ASIO_BLOCK_LEN];

private:
	void SetRenderSilence(bool silence, bool wait=false);

public:
	static int baseChannel;

public:
	CASIODevice(SoundDeviceID id, const std::wstring &internalID);
	~CASIODevice();

public:
	bool InternalOpen();
	bool InternalClose();
	void FillAudioBuffer();
	void InternalReset();
	void InternalStart();
	void InternalStop();
	bool IsOpen() const { return (m_pAsioDrv != NULL); }
	UINT GetNumBuffers() { return 2; }
	float GetCurrentRealLatencyMS() { return m_nAsioBufferLen * 2 * 1000.0f / m_Settings.Samplerate; }

	SoundDeviceCaps GetDeviceCaps(const std::vector<uint32> &baseSampleRates);

public:
	static std::vector<SoundDeviceInfo> EnumerateDevices();

protected:
	void OpenDevice();
	void CloseDevice();

protected:
	void BufferSwitch(long doubleBufferIndex);

	static CASIODevice *gpCurrentAsio;
	static LONG gnFillBuffers;
	static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
	static void SampleRateDidChange(ASIOSampleRate sRate);
	static long AsioMessage(long selector, long value, void* message, double* opt);
	static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	
	void ReportASIOException(const std::string &str);
};

#endif // NO_ASIO

