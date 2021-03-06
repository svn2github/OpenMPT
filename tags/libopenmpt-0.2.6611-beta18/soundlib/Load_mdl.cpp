/*
 * Load_mdl.cpp
 * ------------
 * Purpose: DigiTrakker (MDL) module loader
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

//#define MDL_LOG

#if MPT_COMPILER_MSVC
#pragma warning(disable:4244) //"conversion from 'type1' to 'type2', possible loss of data"
#endif

#ifdef NEEDS_PRAGMA_PACK
#pragma pack(push, 1)
#endif

struct PACKED MDLFileHeader
{
	uint32 id;	// "DMDL" = 0x4C444D44
	uint8  version;
};

STATIC_ASSERT(sizeof(MDLFileHeader) == 5);


struct PACKED MDLInfoBlock
{
	char   songname[32];
	char   composer[20];
	uint16 norders;
	uint16 repeatpos;
	uint8  globalvol;
	uint8  speed;
	uint8  tempo;
	uint8  channelinfo[32];
	uint8  seq[256];
};

STATIC_ASSERT(sizeof(MDLInfoBlock) == 347);


struct PACKED MDLPatternHeader
{
	uint8  channels;
	uint8  lastrow;	// nrows = lastrow+1
	char   name[16];
};

STATIC_ASSERT(sizeof(MDLPatternHeader) == 18);


struct PACKED MDLSampleHeaderCommon
{
	uint8 sampleIndex;
	char  name[32];
	char  filename[8];
};

STATIC_ASSERT(sizeof(MDLSampleHeaderCommon) == 41);


struct PACKED MDLSampleHeader
{
	MDLSampleHeaderCommon info;
	uint32 c4Speed;
	uint32 length;
	uint32 loopStart;
	uint32 loopLength;
	uint8  unused; // was volume in v0.0, why it was changed I have no idea
	uint8  flags;
};

STATIC_ASSERT(sizeof(MDLSampleHeader) == 59);


struct PACKED MDLSampleHeaderv0
{
	MDLSampleHeaderCommon info;
	uint16 c4Speed;
	uint32 length;
	uint32 loopStart;
	uint32 loopLength;
	uint8  volume;
	uint8  flags;
};

STATIC_ASSERT(sizeof(MDLSampleHeaderv0) == 57);


#ifdef NEEDS_PRAGMA_PACK
#pragma pack(pop)
#endif


static void ConvertMDLCommand(ModCommand *m, uint32 eff, uint32 data)
//-------------------------------------------------------------------
{
	uint32 command = 0, param = data;
	switch(eff)
	{
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_ARPEGGIO; break;
	case 0x07:	command = (param < 0x20) ? CMD_SPEED : CMD_TEMPO; break;
	case 0x08:	command = CMD_PANNING8; param <<= 1; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_GLOBALVOLUME; break;
	case 0x0D:	command = CMD_PATTERNBREAK; param = (data & 0x0F) + (data>>4)*10; break;
	case 0x0E:
		command = CMD_S3MCMDEX;
		switch(data & 0xF0)
		{
		case 0x00:	command = 0; break; // What is E0x in MDL (there is a bunch) ?
		case 0x10:	if (param & 0x0F) { param |= 0xF0; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x20:	if (param & 0x0F) { param = (param << 4) | 0x0F; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x30:	param = (data & 0x0F) | 0x10; break; // glissando
		case 0x40:	param = (data & 0x0F) | 0x30; break; // vibrato waveform
		case 0x60:	param = (data & 0x0F) | 0xB0; break;
		case 0x70:	param = (data & 0x0F) | 0x40; break; // tremolo waveform
		case 0x90:	command = CMD_RETRIG; param &= 0x0F; break;
		case 0xA0:	param = (data & 0x0F) << 4; command = CMD_GLOBALVOLSLIDE; break;
		case 0xB0:	param = data & 0x0F; command = CMD_GLOBALVOLSLIDE; break;
		case 0xF0:	param = ((data >> 8) & 0x0F) | 0xA0; break;
		}
		break;
	case 0x0F:	command = CMD_SPEED; break;
	case 0x10:
		if ((param & 0xF0) != 0xE0)
		{
			command = CMD_VOLUMESLIDE;
			if ((param & 0xF0) == 0xF0)
			{
				param = ((param << 4) | 0x0F);
			} else
			{
				param >>= 2;
				if (param > 0xF)
					param = 0xF;
				param <<= 4;
			}
		}
		break;
	case 0x20:
		if ((param & 0xF0) != 0xE0)
		{
			command = CMD_VOLUMESLIDE;
			if ((param & 0xF0) != 0xF0)
			{
				param >>= 2;
				if (param > 0xF)
					param = 0xF;
			}
		}
		break;

	case 0x30:	command = CMD_RETRIG; break;
	case 0x40:	command = CMD_TREMOLO; break;
	case 0x50:	command = CMD_TREMOR; break;
	case 0xEF:	if (param > 0xFF) param = 0xFF; command = CMD_OFFSET; break;
	}
	if (command)
	{
		m->command = command;
		m->param = param;
	}
}


// Convert MDL envelope data (env points and flags)
static void ConvertMDLEnvelope(const unsigned char *pMDLEnv, InstrumentEnvelope *pMPTEnv)
//---------------------------------------------------------------------------------------
{
	uint16 nCurTick = 1;
	pMPTEnv->nNodes = 15;
	for (uint32 nTick = 0; nTick < 15; nTick++)
	{
		if (nTick) nCurTick += pMDLEnv[nTick * 2 + 1];
		pMPTEnv->Ticks[nTick] = nCurTick;
		pMPTEnv->Values[nTick] = pMDLEnv[nTick * 2 + 2];
		if (!pMDLEnv[nTick * 2 + 1]) // last point reached
		{
			pMPTEnv->nNodes = nTick + 1;
			break;
		}
	}
	pMPTEnv->nSustainStart = pMPTEnv->nSustainEnd = pMDLEnv[31] & 0x0F;
	pMPTEnv->dwFlags.set(ENV_SUSTAIN, (pMDLEnv[31] & 0x10) != 0);
	pMPTEnv->dwFlags.set(ENV_LOOP, (pMDLEnv[31] & 0x20) != 0);
	pMPTEnv->nLoopStart = pMDLEnv[32] & 0x0F;
	pMPTEnv->nLoopEnd = pMDLEnv[32] >> 4;
}


static void UnpackMDLTrack(ModCommand *pat, uint32 nChannels, uint32 nRows, uint32 nTrack, const uint8 *lpTracks, uint32 memLength)
//---------------------------------------------------------------------------------------------------------------------------------
{
	ModCommand cmd, *m = pat;
	if(memLength < 2) return;
	uint32 len = *(const_unaligned_ptr_le<uint16>(lpTracks));
	uint32 pos = 0, row = 0, i;
	lpTracks += 2;
	memLength -=2 ;
	for (uint32 ntrk=1; ntrk<nTrack; ntrk++)
	{
		if(memLength < len + 2) return;
		lpTracks += len;
		memLength -= len;
		len = *(const_unaligned_ptr_le<uint16>(lpTracks));
		lpTracks += 2;
		memLength -= 2;
		LimitMax(len, memLength);
	}
	cmd.note = cmd.instr = 0;
	cmd.volcmd = cmd.vol = 0;
	cmd.command = cmd.param = 0;
	while ((row < nRows) && (pos < len))
	{
		uint32 xx;
		uint8 b = lpTracks[pos++];
		xx = b >> 2;
		switch(b & 0x03)
		{
		case 0x01:
			for (i=0; i<=xx; i++)
			{
				if (row) *m = *(m-nChannels);
				m += nChannels;
				row++;
				if (row >= nRows) break;
			}
			break;

		case 0x02:
			if (xx < row) *m = pat[nChannels*xx];
			m += nChannels;
			row++;
			break;

		case 0x03:
			{
				cmd.note = (xx & 0x01) ? lpTracks[pos++] : 0;
				cmd.instr = (xx & 0x02) ? lpTracks[pos++] : 0;
				cmd.volcmd = cmd.vol = 0;
				cmd.command = cmd.param = 0;
				uint32 volume = (xx & 0x04) ? lpTracks[pos++] : 0;
				uint32 commands = (xx & 0x08) ? lpTracks[pos++] : 0;
				uint32 command1 = commands & 0x0F;
				uint32 command2 = commands & 0xF0;
				uint32 param1 = (xx & 0x10) ? lpTracks[pos++] : 0;
				uint32 param2 = (xx & 0x20) ? lpTracks[pos++] : 0;
				if ((command1 == 0x0E) && ((param1 & 0xF0) == 0xF0) && (!command2))
				{
					param1 = ((param1 & 0x0F) << 8) | param2;
					command1 = 0xEF;
					command2 = param2 = 0;
				}
				if (volume)
				{
					cmd.volcmd = VOLCMD_VOLUME;
					cmd.vol = (volume+1) >> 2;
				}
				ConvertMDLCommand(&cmd, command1, param1);
				if ((cmd.command != CMD_SPEED)
				 && (cmd.command != CMD_TEMPO)
				 && (cmd.command != CMD_PATTERNBREAK))
					ConvertMDLCommand(&cmd, command2, param2);
				*m = cmd;
				m += nChannels;
				row++;
			}
			break;

		// Empty Slots
		default:
			row += xx+1;
			m += (xx+1)*nChannels;
			if (row >= nRows) break;
		}
	}
}



bool CSoundFile::ReadMDL(FileReader &file, ModLoadingFlags loadFlags)
//-------------------------------------------------------------------
{
	file.Rewind();
	MDLFileHeader pmsh;
	if(!file.ReadStruct(pmsh)
		|| !file.CanRead(1024)
		|| pmsh.id != LittleEndian(0x4C444D44)
		|| (pmsh.version & 0xF0) > 0x10)
	{
		return false;
	} else if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	file.Rewind();
	const FileReader::off_t dwMemLength = file.GetLength();
	const uint8 *lpStream = file.GetRawData<uint8>();

	uint32 dwMemPos, dwPos, blocklen, dwTrackPos;
	const MDLInfoBlock *pmib;
	uint32 j, norders = 0, npatterns = 0, ntracks = 0;
	uint32 ninstruments = 0, nsamples = 0;
	uint16 block;
	uint16 patterntracks[MAX_PATTERNS*32];
	uint8 smpinfo[MAX_SAMPLES];
	uint8 insvolenv[MAX_INSTRUMENTS];
	uint8 inspanenv[MAX_INSTRUMENTS];
	const uint8 *pvolenv, *ppanenv, *ppitchenv;
	uint32 nvolenv, npanenv, npitchenv;
	std::vector<ROWINDEX> patternLength;

#ifdef MDL_LOG
	Log("MDL v%d.%d\n", pmsh.version>>4, pmsh.version&0x0f);
#endif
	MemsetZero(patterntracks);
	MemsetZero(smpinfo);
	MemsetZero(insvolenv);
	MemsetZero(inspanenv);
	dwMemPos = 5;
	dwTrackPos = 0;
	pvolenv = ppanenv = ppitchenv = NULL;
	nvolenv = npanenv = npitchenv = 0;

	InitializeGlobals(MOD_TYPE_MDL);
	m_SongFlags = SONG_LINEARSLIDES;

	while (dwMemPos+6 < dwMemLength)
	{
		block = *(const_unaligned_ptr_le<uint16>(lpStream+dwMemPos));
		blocklen = *(const_unaligned_ptr_le<uint32>(lpStream+dwMemPos+2));
		dwMemPos += 6;
		if (blocklen > dwMemLength - dwMemPos)
		{
			if (dwMemPos == 11) return false;
			break;
		}
		switch(block)
		{
		// IN: infoblock
		case 0x4E49:
		#ifdef MDL_LOG
			Log("infoblock: %d bytes\n", blocklen);
		#endif
			if(blocklen < sizeof(MDLInfoBlock) - 256)
				return false;
			pmib = (const MDLInfoBlock *)(lpStream+dwMemPos);
			mpt::String::Read<mpt::String::maybeNullTerminated>(m_songName, pmib->songname);
			{
				std::string artist;
				mpt::String::Read<mpt::String::maybeNullTerminated>(artist, pmib->composer);
				m_songArtist = mpt::ToUnicode(mpt::CharsetCP437, artist);
			}

			norders = pmib->norders;
			LimitMax(norders, ORDERINDEX_MAX);
			LimitMax(norders, blocklen - (sizeof(MDLInfoBlock) - 256));
			
			Order.SetRestartPos(pmib->repeatpos);
			m_nDefaultGlobalVolume = pmib->globalvol;
			m_nDefaultTempo.Set(pmib->tempo);
			m_nDefaultSpeed = pmib->speed;
			m_nChannels = 4;
			for (uint8 i=0; i<32; i++)
			{
				ChnSettings[i].Reset();
				ChnSettings[i].nPan = (pmib->channelinfo[i] & 0x7F) << 1;
				if (pmib->channelinfo[i] & 0x80)
					ChnSettings[i].dwFlags.set(CHN_MUTE);
				else
					m_nChannels = i+1;
			}
			Order.ReadFromArray(pmib->seq, norders, 0xFF, 0xFE);
			break;
		// ME: song message
		case 0x454D:
		#ifdef MDL_LOG
			Log("song message: %d bytes\n", blocklen);
		#endif
			if(blocklen)
			{
				m_songMessage.Read(lpStream + dwMemPos, blocklen - 1, SongMessage::leCR);
			}
			break;
		// PA: Pattern Data
		case 0x4150:
		#ifdef MDL_LOG
			Log("pattern data: %d bytes\n", blocklen);
		#endif
			if(!blocklen)
				break;
			npatterns = lpStream[dwMemPos];
			if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
			dwPos = dwMemPos + 1;

			patternLength.assign(npatterns, 64);

			for (uint32 i=0; i<npatterns; i++)
			{
				const_unaligned_ptr_le<uint16> pdata;
				uint32 ch;

				if (pmsh.version > 0)
				{
					if (dwPos + sizeof(MDLPatternHeader) >= dwMemLength) break;
					const MDLPatternHeader *pmpd = (const MDLPatternHeader *)(lpStream + dwPos);
					if (pmpd->channels > 32) break;
					dwPos += sizeof(MDLPatternHeader);
					patternLength[i] = pmpd->lastrow + 1;
					if (m_nChannels < pmpd->channels) m_nChannels = pmpd->channels;
					ch = pmpd->channels;
				} else
				{
					//Patterns[i].Resize(64, false);
					if (m_nChannels < 32) m_nChannels = 32;
					ch = 32;
				}
				if(2 * ch >= dwMemLength - dwPos) break;
				pdata = const_unaligned_ptr_le<uint16>(lpStream + dwPos);
				dwPos += 2 * ch;

				for (j=0; j<ch; j++) if (j<m_nChannels)
				{
					patterntracks[i*32+j] = pdata[j];
				}
			}
			break;
		// TR: Track Data
		case 0x5254:
		#ifdef MDL_LOG
			Log("track data: %d bytes\n", blocklen);
		#endif
			if (dwTrackPos) break;
			ntracks = *(const_unaligned_ptr_le<uint16>(lpStream+dwMemPos));
			dwTrackPos = dwMemPos+2;
			break;
		// II: Instruments
		case 0x4949:
		#ifdef MDL_LOG
			Log("instruments: %d bytes\n", blocklen);
		#endif
			if(!blocklen)
				break;
			ninstruments = lpStream[dwMemPos];
			dwPos = dwMemPos+1;
			for (uint32 remain = blocklen - 1, i=0; i<ninstruments && remain > 2; i++)
			{
				uint32 nins = lpStream[dwPos];
				if ((nins >= MAX_INSTRUMENTS) || (!nins)) break;
				uint32 insLength = 34u + 14u * lpStream[dwPos + 1];
				if(remain < insLength)
					break;
				if (m_nInstruments < nins) m_nInstruments = nins;
				if (!Instruments[nins])
				{
					uint32 note = 12;
					ModInstrument *pIns = AllocateInstrument(nins);
					if(pIns == nullptr)
					{
						break;
					}

					// I give up. better rewrite this crap (or take SchismTracker's MDL loader).
					mpt::String::Read<mpt::String::maybeNullTerminated>(pIns->name, reinterpret_cast<const char *>(lpStream + dwPos + 2), 32);

					for (j=0; j<lpStream[dwPos+1]; j++)
					{
						const uint8 *ps = lpStream+dwPos+34+14*j;
						while ((note < (uint32)(ps[1]+12)) && (note < NOTE_MAX))
						{
							SAMPLEINDEX ismp = ps[0];
							if(ismp < MAX_SAMPLES)
							{
								pIns->Keyboard[note] = ps[0];
								Samples[ismp].nVolume = ps[2];
								Samples[ismp].nPan = ps[4] << 1;
								Samples[ismp].nVibType = ps[11];
								Samples[ismp].nVibSweep = ps[10];
								Samples[ismp].nVibDepth = ps[9];
								Samples[ismp].nVibRate = ps[8];
							}
							pIns->nFadeOut = (ps[7] << 8) | ps[6];
							if (pIns->nFadeOut == 0xFFFF) pIns->nFadeOut = 0;
							note++;
						}
						// Use volume envelope ?
						if (ps[3] & 0x80)
						{
							pIns->VolEnv.dwFlags.set(ENV_ENABLED);
							insvolenv[nins] = (ps[3] & 0x3F) + 1;
						}
						// Use panning envelope ?
						if (ps[5] & 0x80)
						{
							pIns->PanEnv.dwFlags.set(ENV_ENABLED);
							inspanenv[nins] = (ps[5] & 0x3F) + 1;
						}

						// taken from load_xm.cpp - seems to fix wakingup.mdl
						if(!pIns->VolEnv.dwFlags[ENV_ENABLED] && !pIns->nFadeOut)
							pIns->nFadeOut = 8192;
					}
				}
				remain -= insLength;
				dwPos += insLength;
			}
			for (j=1; j<=m_nInstruments; j++) if (!Instruments[j])
			{
				AllocateInstrument(j);
			}
			break;
		// VE: Volume Envelope
		case 0x4556:
		#ifdef MDL_LOG
			Log("volume envelope: %d bytes\n", blocklen);
		#endif
			if (pvolenv != nullptr || (nvolenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + nvolenv*33 + 1 <= dwMemLength) pvolenv = lpStream + dwMemPos + 1;
			break;
		// PE: Panning Envelope
		case 0x4550:
		#ifdef MDL_LOG
			Log("panning envelope: %d bytes\n", blocklen);
		#endif
			if (ppanenv != nullptr || (npanenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npanenv*33 + 1 <= dwMemLength) ppanenv = lpStream + dwMemPos + 1;
			break;
		// FE: Pitch Envelope
		case 0x4546:
		#ifdef MDL_LOG
			Log("pitch envelope: %d bytes\n", blocklen);
		#endif
			if (ppitchenv != nullptr || (npitchenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npitchenv*33 + 1 <= dwMemLength) ppitchenv = lpStream + dwMemPos + 1;
			break;
		// IS: Sample Infoblock
		case 0x5349:
		#ifdef MDL_LOG
			Log("sample infoblock: %d bytes\n", blocklen);
		#endif
			if(nsamples)
				break;
			nsamples = std::min<uint32>(lpStream[dwMemPos], blocklen / ((pmsh.version > 0) ? sizeof(MDLSampleHeader) : sizeof(MDLSampleHeaderv0)));
			dwPos = dwMemPos + 1;
			for (uint32 i = 0; i < nsamples; i++, dwPos += (pmsh.version > 0) ? sizeof(MDLSampleHeader) : sizeof(MDLSampleHeaderv0))
			{
				const MDLSampleHeaderCommon *info = reinterpret_cast<const MDLSampleHeaderCommon *>(lpStream + dwPos);
				if(!IsInRange(info->sampleIndex, 1, MAX_SAMPLES-1))
				{
					continue;
				}

				if(m_nSamples < info->sampleIndex)
				{
					m_nSamples = info->sampleIndex;
				}
				ModSample &sample = Samples[info->sampleIndex];

				mpt::String::Read<mpt::String::maybeNullTerminated>(m_szNames[info->sampleIndex], info->name);
				mpt::String::Read<mpt::String::maybeNullTerminated>(sample.filename, info->filename);

				if(pmsh.version > 0)
				{
					const MDLSampleHeader *sampleHeader = reinterpret_cast<const MDLSampleHeader *>(lpStream + dwPos);

					sample.nC5Speed = LittleEndian(sampleHeader->c4Speed) * 2;
					sample.nLength = LittleEndian(sampleHeader->length);
					sample.nLoopStart = LittleEndian(sampleHeader->loopStart);
					sample.nLoopEnd = sample.nLoopStart + LittleEndian(sampleHeader->loopLength);
					if(sample.nLoopEnd > sample.nLoopStart)
					{
						sample.uFlags |= CHN_LOOP;
						if((sampleHeader->flags & 0x02))
						{
							sample.uFlags |= CHN_PINGPONGLOOP;
						}
					}
					sample.nGlobalVol = 64;
					//sample.nVolume = 256;

					if((sampleHeader->flags & 0x01))
					{
						sample.uFlags |= CHN_16BIT;
						sample.nLength /= 2;
						sample.nLoopStart /= 2;
						sample.nLoopEnd /= 2;
					}

					smpinfo[info->sampleIndex] = (sampleHeader->flags >> 2) & 3;
				} else
				{
					const MDLSampleHeaderv0 *sampleHeader = reinterpret_cast<const MDLSampleHeaderv0 *>(lpStream + dwPos);

					sample.nC5Speed = LittleEndianW(sampleHeader->c4Speed) * 2;
					sample.nLength = LittleEndian(sampleHeader->length);
					sample.nLoopStart = LittleEndian(sampleHeader->loopStart);
					sample.nLoopEnd = sample.nLoopStart + LittleEndian(sampleHeader->loopLength);
					if(sample.nLoopEnd > sample.nLoopStart)
					{
						sample.uFlags |= CHN_LOOP;
						if((sampleHeader->flags & 0x02))
						{
							sample.uFlags |= CHN_PINGPONGLOOP;
						}
					}
					sample.nGlobalVol = 64;
					sample.nVolume = sampleHeader->volume;

					if((sampleHeader->flags & 0x01))
					{
						sample.uFlags |= CHN_16BIT;
						sample.nLength /= 2;
						sample.nLoopStart /= 2;
						sample.nLoopEnd /= 2;
					}

					smpinfo[info->sampleIndex] = (sampleHeader->flags >> 2) & 3;
				}
			}
			break;
		// SA: Sample Data
		case 0x4153:
		#ifdef MDL_LOG
			Log("sample data: %d bytes\n", blocklen);
		#endif
			if(!(loadFlags & loadSampleData))
			{
				break;
			}
			dwPos = dwMemPos;
			for (uint32 i=1; i<=m_nSamples; i++) if ((Samples[i].nLength) && (!Samples[i].pSample) && (smpinfo[i] != 3) && (dwPos < dwMemLength))
			{
				ModSample &sample = Samples[i];
				SampleIO sampleIO(
					(sample.uFlags & CHN_16BIT) ? SampleIO::_16bit : SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::signedPCM);

				if (!smpinfo[i])
				{
					FileReader chunk(mpt::as_span(lpStream + dwPos, dwMemLength - dwPos));
					dwPos += sampleIO.ReadSample(sample, chunk);
				} else if(dwPos < dwMemLength - 4)
				{
					uint32 dwLen = *(const_unaligned_ptr_le<uint32>(lpStream+dwPos));
					dwPos += 4;
					if ( (dwLen <= dwMemLength) && (dwPos <= dwMemLength - dwLen) && (dwLen > 4) )
					{
						sampleIO |= SampleIO::MDL;
						FileReader chunk(mpt::as_span(lpStream + dwPos , dwLen));
						sampleIO.ReadSample(sample, chunk);
					}
					dwPos += dwLen;
				}
			}
			break;
		#ifdef MDL_LOG
		default:
			Log("unknown block (%c%c): %d bytes\n", block&0xff, block>>8, blocklen);
		#endif
		}
		dwMemPos += blocklen;
	}
	// Unpack Patterns
	if((loadFlags & loadPatternData) && (dwTrackPos) && (npatterns) && (m_nChannels) && (ntracks))
	{
		for (uint32 ipat=0; ipat<npatterns; ipat++)
		{
			if(!Patterns.Insert(ipat, patternLength[ipat]))
			{
				break;
			}
			for (uint32 chn=0; chn<m_nChannels; chn++) if ((patterntracks[ipat*32+chn]) && (patterntracks[ipat*32+chn] <= ntracks))
			{
				ModCommand *m = Patterns[ipat] + chn;
				UnpackMDLTrack(m, m_nChannels, Patterns[ipat].GetNumRows(), patterntracks[ipat*32+chn], lpStream + dwTrackPos, dwMemLength - dwTrackPos);
			}
		}
	}
	// Set up envelopes
	for (uint32 iIns=1; iIns<=m_nInstruments; iIns++) if (Instruments[iIns])
	{
		// Setup volume envelope
		if ((nvolenv) && (pvolenv) && (insvolenv[iIns]))
		{
			const uint8 *pve = pvolenv;
			for (uint32 nve = 0; nve < nvolenv; nve++, pve += 33)
			{
				if (pve[0] + 1 == insvolenv[iIns])
					ConvertMDLEnvelope(pve, &Instruments[iIns]->VolEnv);
			}
		}
		// Setup panning envelope
		if ((npanenv) && (ppanenv) && (inspanenv[iIns]))
		{
			const uint8 *ppe = ppanenv;
			for (uint32 npe = 0; npe < npanenv; npe++, ppe += 33)
			{
				if (ppe[0] + 1 == inspanenv[iIns])
					ConvertMDLEnvelope(ppe, &Instruments[iIns]->PanEnv);
			}
		}
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////
// MDL Sample Unpacking

// MDL Huffman ReadBits compression
uint16 MDLReadBits(uint32 &bitbuf, uint32 &bitnum, const uint8 *(&ibuf), size_t &bytesLeft, int8 n)
//-------------------------------------------------------------------------------------------------
{
	uint16 v = (uint16)(bitbuf & ((1 << n) - 1) );
	bitbuf >>= n;
	bitnum -= n;
	if (bitnum <= 24)
	{
		if(!bytesLeft)
		{
			bitnum += 8;
			return uint16_max;
		}
		bitbuf |= (((uint32)(*ibuf++)) << bitnum);
		bitnum += 8;
		bytesLeft--;
	}
	return v;
}


OPENMPT_NAMESPACE_END
