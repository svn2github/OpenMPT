/*
 * Load_dsm.cpp
 * ------------
 * Purpose: DSIK Internal Format (DSM) module loader
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#pragma pack(push, 1)

#define DSMID_RIFF	0x46464952	// "RIFF"
#define DSMID_DSMF	0x464d5344	// "DSMF"
#define DSMID_SONG	0x474e4f53	// "SONG"
#define DSMID_INST	0x54534e49	// "INST"
#define DSMID_PATT	0x54544150	// "PATT"


typedef struct DSMNOTE
{
	BYTE note,ins,vol,cmd,inf;
} DSMNOTE;


typedef struct DSMSAMPLE
{
	DWORD id_INST;
	DWORD inst_len;
	CHAR filename[13];
	BYTE flags;
	BYTE flags2;
	BYTE volume;
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	DWORD reserved1;
	WORD c2spd;
	WORD reserved2;
	CHAR samplename[28];
} DSMSAMPLE;


typedef struct DSMFILEHEADER
{
	DWORD id_RIFF;	// "RIFF"
	DWORD riff_len;
	DWORD id_DSMF;	// "DSMF"
	DWORD id_SONG;	// "SONG"
	DWORD song_len;
} DSMFILEHEADER;


typedef struct DSMSONG
{
	CHAR songname[28];
	WORD reserved1;
	WORD flags;
	DWORD reserved2;
	WORD numord;
	WORD numsmp;
	WORD numpat;
	WORD numtrk;
	BYTE globalvol;
	BYTE mastervol;
	BYTE speed;
	BYTE bpm;
	BYTE panpos[16];
	BYTE orders[128];
} DSMSONG;

typedef struct DSMPATT
{
	DWORD id_PATT;
	DWORD patt_len;
	BYTE dummy1;
	BYTE dummy2;
} DSMPATT;

#pragma pack(pop)


bool CSoundFile::ReadDSM(const LPCBYTE lpStream, const DWORD dwMemLength)
//-----------------------------------------------------------------------
{
	DSMFILEHEADER *pfh = (DSMFILEHEADER *)lpStream;
	DSMSONG *psong;
	DWORD dwMemPos;
	UINT nSmp;
	PATTERNINDEX nPat;

	if ((!lpStream) || (dwMemLength < 1024) || (pfh->id_RIFF != DSMID_RIFF)
	 || (pfh->riff_len + 8 > dwMemLength) || (pfh->riff_len < 1024)
	 || (pfh->id_DSMF != DSMID_DSMF) || (pfh->id_SONG != DSMID_SONG)
	 || (pfh->song_len > dwMemLength)) return false;
	psong = (DSMSONG *)(lpStream + sizeof(DSMFILEHEADER));
	dwMemPos = sizeof(DSMFILEHEADER) + pfh->song_len;
	m_nType = MOD_TYPE_DSM;
	m_nChannels = psong->numtrk;
	if (m_nChannels < 1) m_nChannels = 1;
	if (m_nChannels > 16) m_nChannels = 16;
	m_nSamples = psong->numsmp;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES - 1;
	m_nDefaultSpeed = psong->speed;
	m_nDefaultTempo = psong->bpm;
	m_nDefaultGlobalVolume = psong->globalvol << 2;
	if ((!m_nDefaultGlobalVolume) || (m_nDefaultGlobalVolume > MAX_GLOBAL_VOLUME)) m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	m_nSamplePreAmp = psong->mastervol & 0x7F;
	Order.ReadFromArray(psong->orders, psong->numord);

	for (UINT iPan=0; iPan<16; iPan++)
	{
		ChnSettings[iPan].nPan = 0x80;
		if (psong->panpos[iPan] <= 0x80)
		{
			ChnSettings[iPan].nPan = psong->panpos[iPan] << 1;
		}
	}

	StringFixer::ReadString<StringFixer::maybeNullTerminated>(m_szNames[0], psong->songname);

	nPat = 0;
	nSmp = 1;
	while (dwMemPos < dwMemLength - 8)
	{
		DSMPATT *ppatt = (DSMPATT *)(lpStream + dwMemPos);
		DSMSAMPLE *pSmp = (DSMSAMPLE *)(lpStream+dwMemPos);
		// Reading Patterns
		if (ppatt->id_PATT == DSMID_PATT)
		{
			dwMemPos += 8;
			if (dwMemPos + ppatt->patt_len >= dwMemLength) break;
			DWORD dwPos = dwMemPos;
			dwMemPos += ppatt->patt_len;
			if(Patterns.Insert(nPat, 64))
				break;

			ModCommand *m = Patterns[nPat];
			UINT row = 0;
			while ((row < 64) && (dwPos + 2 <= dwMemPos))
			{
				UINT flag = lpStream[dwPos++];
				if (flag)
				{
					UINT ch = (flag & 0x0F) % m_nChannels;
					if (flag & 0x80)
					{
						UINT note = lpStream[dwPos++];
						if (note)
						{
							if (note <= 12*9) note += 12;
							m[ch].note = (BYTE)note;
						}
					}
					if (flag & 0x40)
					{
						m[ch].instr = lpStream[dwPos++];
					}
					if (flag & 0x20)
					{
						m[ch].volcmd = VOLCMD_VOLUME;
						m[ch].vol = lpStream[dwPos++];
					}
					if (flag & 0x10)
					{
						UINT command = lpStream[dwPos++];
						UINT param = lpStream[dwPos++];
						switch(command)
						{
						// 4-bit Panning
						case 0x08:
							switch(param & 0xF0)
							{
							case 0x00: param <<= 4; break;
							case 0x10: command = 0x0A; param = (param & 0x0F) << 4; break;
							case 0x20: command = 0x0E; param = (param & 0x0F) | 0xA0; break;
							case 0x30: command = 0x0E; param = (param & 0x0F) | 0x10; break;
							case 0x40: command = 0x0E; param = (param & 0x0F) | 0x20; break;
							default: command = 0;
							}
							break;
						// Portamentos
						case 0x11:
						case 0x12:
							command &= 0x0F;
							break;
						// 3D Sound (?)
						case 0x13:
							command = 'X' - 55;
							param = 0x91;
							break;
						default:
							// Volume + Offset (?)
							command = ((command & 0xF0) == 0x20) ? 0x09 : 0;
						}
						m[ch].command = (BYTE)command;
						m[ch].param = (BYTE)param;
						if (command) ConvertModCommand(m[ch]);
					}
				} else
				{
					m += m_nChannels;
					row++;
				}
			}
			nPat++;
		} else
		// Reading Samples
		if ((nSmp <= m_nSamples) && (pSmp->id_INST == DSMID_INST))
		{
			if (dwMemPos + pSmp->inst_len >= dwMemLength - 8) break;
			DWORD dwPos = dwMemPos + sizeof(DSMSAMPLE);
			dwMemPos += 8 + pSmp->inst_len;

			ModSample &sample = Samples[nSmp];
			sample.Initialize();

			StringFixer::ReadString<StringFixer::maybeNullTerminated>(m_szNames[nSmp], pSmp->samplename);
			StringFixer::ReadString<StringFixer::nullTerminated>(sample.filename, pSmp->filename);

			sample.nC5Speed = pSmp->c2spd;
			sample.uFlags.set(CHN_LOOP, (pSmp->flags & 1) != 0);
			sample.nLength = pSmp->length;
			sample.nLoopStart = pSmp->loopstart;
			sample.nLoopEnd = pSmp->loopend;
			sample.nVolume = (WORD)(pSmp->volume << 2);
			if (sample.nVolume > 256) sample.nVolume = 256;

			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				(pSmp->flags & 2) ? SampleIO::signedPCM : SampleIO::unsignedPCM)
				.ReadSample(sample, (LPCSTR)(lpStream + dwPos), dwMemLength - dwPos);

			nSmp++;
		} else
		{
			break;
		}
	}
	return true;
}
