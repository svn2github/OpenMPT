/*
 * This source code is public domain.
 *
 * Copied to OpenMPT from libmodplug.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// DigiTracker (MDL) module loader          //
//////////////////////////////////////////////
#include "stdafx.h"
#include "sndfile.h"

//#define MDL_LOG

#pragma warning(disable:4244) //"conversion from 'type1' to 'type2', possible loss of data"

typedef struct MDLSONGHEADER
{
	DWORD id;	// "DMDL" = 0x4C444D44
	BYTE version;
} MDLSONGHEADER;


typedef struct MDLINFOBLOCK
{
	CHAR songname[32];
	CHAR composer[20];
	WORD norders;
	WORD repeatpos;
	BYTE globalvol;
	BYTE speed;
	BYTE tempo;
	BYTE channelinfo[32];
	BYTE seq[256];
} MDLINFOBLOCK;


typedef struct MDLPATTERNDATA
{
	BYTE channels;
	BYTE lastrow;	// nrows = lastrow+1
	CHAR name[16];
	WORD data[1];
} MDLPATTERNDATA;


void ConvertMDLCommand(MODCOMMAND *m, UINT eff, UINT data)
//--------------------------------------------------------
{
	UINT command = 0, param = data;
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
	case 0x10:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) == 0xF0) param = ((param << 4) | 0x0F); else param >>= 2; } break;
	case 0x20:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) != 0xF0) param >>= 2; } break;
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


void UnpackMDLTrack(MODCOMMAND *pat, UINT nChannels, UINT nRows, UINT nTrack, const BYTE *lpTracks)
//-------------------------------------------------------------------------------------------------
{
	MODCOMMAND cmd, *m = pat;
	UINT len = *((WORD *)lpTracks);
	UINT pos = 0, row = 0, i;
	lpTracks += 2;
	for (UINT ntrk=1; ntrk<nTrack; ntrk++)
	{
		lpTracks += len;
		len = *((WORD *)lpTracks);
		lpTracks += 2;
	}
	cmd.note = cmd.instr = 0;
	cmd.volcmd = cmd.vol = 0;
	cmd.command = cmd.param = 0;
	while ((row < nRows) && (pos < len))
	{
		UINT xx;
		BYTE b = lpTracks[pos++];
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
				if ((cmd.note < NOTE_MAX-12) && (cmd.note)) cmd.note += 12;
				UINT volume = (xx & 0x04) ? lpTracks[pos++] : 0;
				UINT commands = (xx & 0x08) ? lpTracks[pos++] : 0;
				UINT command1 = commands & 0x0F;
				UINT command2 = commands & 0xF0;
				UINT param1 = (xx & 0x10) ? lpTracks[pos++] : 0;
				UINT param2 = (xx & 0x20) ? lpTracks[pos++] : 0;
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



bool CSoundFile::ReadMDL(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	DWORD dwMemPos, dwPos, blocklen, dwTrackPos;
	const MDLSONGHEADER *pmsh = (const MDLSONGHEADER *)lpStream;
	MDLINFOBLOCK *pmib;
	UINT i,j, norders = 0, npatterns = 0, ntracks = 0;
	UINT ninstruments = 0, nsamples = 0;
	WORD block;
	WORD patterntracks[MAX_PATTERNS*32];
	BYTE smpinfo[MAX_SAMPLES];
	BYTE insvolenv[MAX_INSTRUMENTS];
	BYTE inspanenv[MAX_INSTRUMENTS];
	LPCBYTE pvolenv, ppanenv, ppitchenv;
	UINT nvolenv, npanenv, npitchenv;

	if ((!lpStream) || (dwMemLength < 1024)) return false;
	if ((pmsh->id != 0x4C444D44) || ((pmsh->version & 0xF0) > 0x10)) return false;
#ifdef MDL_LOG
	Log("MDL v%d.%d\n", pmsh->version>>4, pmsh->version&0x0f);
#endif
	memset(patterntracks, 0, sizeof(patterntracks));
	memset(smpinfo, 0, sizeof(smpinfo));
	memset(insvolenv, 0, sizeof(insvolenv));
	memset(inspanenv, 0, sizeof(inspanenv));
	dwMemPos = 5;
	dwTrackPos = 0;
	pvolenv = ppanenv = ppitchenv = NULL;
	nvolenv = npanenv = npitchenv = 0;
	m_nSamples = m_nInstruments = 0;
	while (dwMemPos+6 < dwMemLength)
	{
		block = *((WORD *)(lpStream+dwMemPos));
		blocklen = *((DWORD *)(lpStream+dwMemPos+2));
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
			pmib = (MDLINFOBLOCK *)(lpStream+dwMemPos);
			memcpy(m_szNames[0], pmib->songname, 31);
			norders = pmib->norders;
			if (norders > MAX_ORDERS) norders = MAX_ORDERS;
			m_nRestartPos = pmib->repeatpos;
			m_nDefaultGlobalVolume = pmib->globalvol;
			m_nDefaultTempo = pmib->tempo;
			m_nDefaultSpeed = pmib->speed;
			m_nChannels = 4;
			for (i=0; i<32; i++)
			{
				ChnSettings[i].nVolume = 64;
				ChnSettings[i].nPan = (pmib->channelinfo[i] & 0x7F) << 1;
				if (pmib->channelinfo[i] & 0x80)
					ChnSettings[i].dwFlags |= CHN_MUTE;
				else
					m_nChannels = i+1;
			}
			Order.ReadAsByte(pmib->seq, norders, sizeof(pmib->seq));
			break;
		// ME: song message
		case 0x454D:
		#ifdef MDL_LOG
			Log("song message: %d bytes\n", blocklen);
		#endif
			if (blocklen)
			{
				if (m_lpszSongComments) delete[] m_lpszSongComments;
				m_lpszSongComments = new char[blocklen];
				if (m_lpszSongComments)
				{
					memcpy(m_lpszSongComments, lpStream+dwMemPos, blocklen);
					m_lpszSongComments[blocklen-1] = 0;
				}
			}
			break;
		// PA: Pattern Data
		case 0x4150:
		#ifdef MDL_LOG
			Log("pattern data: %d bytes\n", blocklen);
		#endif
			npatterns = lpStream[dwMemPos];
			if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
			dwPos = dwMemPos + 1;
			for (i=0; i<npatterns; i++)
			{
				const WORD *pdata;
				UINT ch;

				if (dwPos+18 >= dwMemLength) break;
				if (pmsh->version > 0)
				{
					const MDLPATTERNDATA *pmpd = (const MDLPATTERNDATA *)(lpStream + dwPos);
					if (pmpd->channels > 32) break;
					Patterns[i].Resize(pmpd->lastrow+1);
					if (m_nChannels < pmpd->channels) m_nChannels = pmpd->channels;
					dwPos += 18 + 2*pmpd->channels;
					pdata = pmpd->data;
					ch = pmpd->channels;
				} else
				{
					pdata = (const WORD *)(lpStream + dwPos);
					Patterns[i].Resize(64);
					if (m_nChannels < 32) m_nChannels = 32;
					dwPos += 2*32;
					ch = 32;
				}
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
			ntracks = *((WORD *)(lpStream+dwMemPos));
			dwTrackPos = dwMemPos+2;
			break;
		// II: Instruments
		case 0x4949:
		#ifdef MDL_LOG
			Log("instruments: %d bytes\n", blocklen);
		#endif
			ninstruments = lpStream[dwMemPos];
			dwPos = dwMemPos+1;
			for (i=0; i<ninstruments; i++)
			{
				UINT nins = lpStream[dwPos];
				if ((nins >= MAX_INSTRUMENTS) || (!nins)) break;
				if (m_nInstruments < nins) m_nInstruments = nins;
				if (!Instruments[nins])
				{
					UINT note = 12;
					if ((Instruments[nins] = new MODINSTRUMENT) == NULL) break;
					MODINSTRUMENT *pIns = Instruments[nins];
					memset(pIns, 0, sizeof(MODINSTRUMENT));
					memcpy(pIns->name, lpStream+dwPos+2, 32);
					pIns->nGlobalVol = 64;
					pIns->nPPC = 5*12;
					SetDefaultInstrumentValues(pIns);
					for (j=0; j<lpStream[dwPos+1]; j++)
					{
						const BYTE *ps = lpStream+dwPos+34+14*j;
						while ((note < (UINT)(ps[1]+12)) && (note < NOTE_MAX))
						{
							pIns->NoteMap[note] = note+1;
							if (ps[0] < MAX_SAMPLES)
							{
								int ismp = ps[0];
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
							pIns->dwFlags |= ENV_VOLUME;
							insvolenv[nins] = (ps[3] & 0x3F) + 1;
						}
						// Use panning envelope ?
						if (ps[5] & 0x80)
						{
							pIns->dwFlags |= ENV_PANNING;
							inspanenv[nins] = (ps[5] & 0x3F) + 1;
						}
					}
				}
				dwPos += 34 + 14*lpStream[dwPos+1];
			}
			for (j=1; j<=m_nInstruments; j++) if (!Instruments[j])
			{
				Instruments[j] = new MODINSTRUMENT;
				if (Instruments[j]) memset(Instruments[j], 0, sizeof(MODINSTRUMENT));
			}
			break;
		// VE: Volume Envelope
		case 0x4556:
		#ifdef MDL_LOG
			Log("volume envelope: %d bytes\n", blocklen);
		#endif
			if ((nvolenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + nvolenv*32 + 1 <= dwMemLength) pvolenv = lpStream + dwMemPos + 1;
			break;
		// PE: Panning Envelope
		case 0x4550:
		#ifdef MDL_LOG
			Log("panning envelope: %d bytes\n", blocklen);
		#endif
			if ((npanenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npanenv*32 + 1 <= dwMemLength) ppanenv = lpStream + dwMemPos + 1;
			break;
		// FE: Pitch Envelope
		case 0x4546:
		#ifdef MDL_LOG
			Log("pitch envelope: %d bytes\n", blocklen);
		#endif
			if ((npitchenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npitchenv*32 + 1 <= dwMemLength) ppitchenv = lpStream + dwMemPos + 1;
			break;
		// IS: Sample Infoblock
		case 0x5349:
		#ifdef MDL_LOG
			Log("sample infoblock: %d bytes\n", blocklen);
		#endif
			nsamples = lpStream[dwMemPos];
			dwPos = dwMemPos+1;
			for (i=0; i<nsamples; i++, dwPos += (pmsh->version > 0) ? 59 : 57)
			{
				UINT nins = lpStream[dwPos];
				if ((nins >= MAX_SAMPLES) || (!nins)) continue;
				if (m_nSamples < nins) m_nSamples = nins;
				MODSAMPLE *pSmp = &Samples[nins];
				memcpy(m_szNames[nins], lpStream+dwPos+1, 31);
				memcpy(pSmp->filename, lpStream+dwPos+33, 8);
				const BYTE *p = lpStream+dwPos+41;
				if (pmsh->version > 0)
				{
					pSmp->nC5Speed = *((DWORD *)p);
					p += 4;
				} else
				{
					pSmp->nC5Speed = *((WORD *)p);
					p += 2;
				}
				pSmp->nLength = *((DWORD *)(p));
				pSmp->nLoopStart = *((DWORD *)(p+4));
				pSmp->nLoopEnd = pSmp->nLoopStart + *((DWORD *)(p+8));
				if (pSmp->nLoopEnd > pSmp->nLoopStart) pSmp->uFlags |= CHN_LOOP;
				pSmp->nVolume = 256;
				pSmp->nGlobalVol = 64;
				if (p[13] & 0x01)
				{
					pSmp->uFlags |= CHN_16BIT;
					pSmp->nLength >>= 1;
					pSmp->nLoopStart >>= 1;
					pSmp->nLoopEnd >>= 1;
				}
				if (p[13] & 0x02) pSmp->uFlags |= CHN_PINGPONGLOOP;
				smpinfo[nins] = (p[13] >> 2) & 3;
			}
			break;
		// SA: Sample Data
		case 0x4153:
		#ifdef MDL_LOG
			Log("sample data: %d bytes\n", blocklen);
		#endif
			dwPos = dwMemPos;
			for (i=1; i<=m_nSamples; i++) if ((Samples[i].nLength) && (!Samples[i].pSample) && (smpinfo[i] != 3) && (dwPos < dwMemLength))
			{
				MODSAMPLE *pSmp = &Samples[i];
				UINT flags = (pSmp->uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
				if (!smpinfo[i])
				{
					dwPos += ReadSample(pSmp, flags, (LPSTR)(lpStream+dwPos), dwMemLength - dwPos);
				} else
				{
					DWORD dwLen = *((DWORD *)(lpStream+dwPos));
					dwPos += 4;
					if ( (dwLen <= dwMemLength) && (dwPos <= dwMemLength - dwLen) && (dwLen > 4) )
					{
						flags = (pSmp->uFlags & CHN_16BIT) ? RS_MDL16 : RS_MDL8;
						ReadSample(pSmp, flags, (LPSTR)(lpStream+dwPos), dwLen);
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
	if ((dwTrackPos) && (npatterns) && (m_nChannels) && (ntracks))
	{
		for (UINT ipat=0; ipat<npatterns; ipat++)
		{
			if ((Patterns[ipat] = AllocatePattern(PatternSize[ipat], m_nChannels)) == NULL) break;
			for (UINT chn=0; chn<m_nChannels; chn++) if ((patterntracks[ipat*32+chn]) && (patterntracks[ipat*32+chn] <= ntracks))
			{
				MODCOMMAND *m = Patterns[ipat] + chn;
				UnpackMDLTrack(m, m_nChannels, PatternSize[ipat], patterntracks[ipat*32+chn], lpStream+dwTrackPos);
			}
		}
	}
	// Set up envelopes
	for (UINT iIns=1; iIns<=m_nInstruments; iIns++) if (Instruments[iIns])
	{
		MODINSTRUMENT *pIns = Instruments[iIns];
		// Setup volume envelope
		if ((nvolenv) && (pvolenv) && (insvolenv[iIns]))
		{
			LPCBYTE pve = pvolenv;
			for (UINT nve=0; nve<nvolenv; nve++, pve+=33) if (pve[0]+1 == insvolenv[iIns])
			{
				WORD vtick = 1;
				pIns->VolEnv.nNodes = 15;
				for (UINT iv=0; iv<15; iv++)
				{
					if (iv) vtick += pve[iv*2+1];
					pIns->VolEnv.Ticks[iv] = vtick;
					pIns->VolEnv.Values[iv] = pve[iv*2+2];
					if (!pve[iv*2+1])
					{
						pIns->VolEnv.nNodes = iv+1;
						break;
					}
				}
				pIns->VolEnv.nSustainStart = pIns->VolEnv.nSustainEnd = pve[31] & 0x0F;
				if (pve[31] & 0x10) pIns->dwFlags |= ENV_VOLSUSTAIN;
				if (pve[31] & 0x20) pIns->dwFlags |= ENV_VOLLOOP;
				pIns->VolEnv.nLoopStart = pve[32] & 0x0F;
				pIns->VolEnv.nLoopEnd = pve[32] >> 4;
			}
		}
		// Setup panning envelope
		if ((npanenv) && (ppanenv) && (inspanenv[iIns]))
		{
			LPCBYTE ppe = ppanenv;
			for (UINT npe=0; npe<npanenv; npe++, ppe+=33) if (ppe[0]+1 == inspanenv[iIns])
			{
				WORD vtick = 1;
				pIns->PanEnv.nNodes = 15;
				for (UINT iv=0; iv<15; iv++)
				{
					if (iv) vtick += ppe[iv*2+1];
					pIns->PanEnv.Ticks[iv] = vtick;
					pIns->PanEnv.Values[iv] = ppe[iv*2+2];
					if (!ppe[iv*2+1])
					{
						pIns->PanEnv.nNodes = iv+1;
						break;
					}
				}
				if (ppe[31] & 0x10) pIns->dwFlags |= ENV_PANSUSTAIN;
				if (ppe[31] & 0x20) pIns->dwFlags |= ENV_PANLOOP;
				pIns->PanEnv.nLoopStart = ppe[32] & 0x0F;
				pIns->PanEnv.nLoopEnd = ppe[32] >> 4;
			}
		}
	}
	m_dwSongFlags |= SONG_LINEARSLIDES;
	m_nType = MOD_TYPE_MDL;
	return true;
}


/////////////////////////////////////////////////////////////////////////
// MDL Sample Unpacking

// MDL Huffman ReadBits compression
WORD MDLReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n)
//-----------------------------------------------------------------
{
	WORD v = (WORD)(bitbuf & ((1 << n) - 1) );
	bitbuf >>= n;
	bitnum -= n;
	if (bitnum <= 24)
	{
		bitbuf |= (((DWORD)(*ibuf++)) << bitnum);
		bitnum += 8;
	}
	return v;
}


