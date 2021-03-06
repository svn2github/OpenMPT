/*
 * Load_imf.cpp
 * ------------
 * Purpose: IMF (Imago Orpheus) module loader
 * Notes  : Reverb and Chorus are not supported.
 * Authors: Storlek (Original author - http://schismtracker.org/ - code ported with permission)
 *			Johannes Schultz (OpenMPT Port, tweaks)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/moddoc.h"
#endif // MODPLUG_TRACKER

#pragma pack(push, 1)

struct IMFCHANNEL
{
	char name[12];	// Channel name (ASCIIZ-String, max 11 chars)
	uint8 chorus;	// Default chorus
	uint8 reverb;	// Default reverb
	uint8 panning;	// Pan positions 00-FF
	uint8 status;	// Channel status: 0 = enabled, 1 = mute, 2 = disabled (ignore effects!)
};

struct IMFHEADER
{
	char title[32];				// Songname (ASCIIZ-String, max. 31 chars)
	uint16 ordnum;				// Number of orders saved
	uint16 patnum;				// Number of patterns saved
	uint16 insnum;				// Number of instruments saved
	uint16 flags;				// Module flags (&1 => linear)
	uint8 unused1[8];
	uint8 tempo;				// Default tempo (Axx, 1..255)
	uint8 bpm;					// Default beats per minute (BPM) (Txx, 32..255)
	uint8 master;				// Default mastervolume (Vxx, 0..64)
	uint8 amp;					// Amplification factor (mixing volume, 4..127)
	uint8 unused2[8];
	char im10[4];				// 'IM10'
	IMFCHANNEL channels[32];	// Channel settings
	uint8 orderlist[256];		// Order list (0xFF = +++; blank out anything beyond ordnum)
};

enum
{
	IMF_ENV_VOL = 0,
	IMF_ENV_PAN = 1,
	IMF_ENV_FILTER = 2,
};

struct IMFENVELOPE
{
	uint8 points;		// Number of envelope points
	uint8 sustain;		// Envelope sustain point
	uint8 loop_start;	// Envelope loop start point
	uint8 loop_end;		// Envelope loop end point
	uint8 flags;		// Envelope flags
	uint8 unused[3];
};

struct IMFENVNODES
{
	uint16 tick;
	uint16 value;
};

struct IMFINSTRUMENT
{
	char name[32];		// Inst. name (ASCIIZ-String, max. 31 chars)
	uint8 map[120];		// Multisample settings
	uint8 unused[8];
	IMFENVNODES nodes[3][16];
	IMFENVELOPE env[3];
	uint16 fadeout;		// Fadeout rate (0...0FFFH)
	uint16 smpnum;		// Number of samples in instrument
	char ii10[4];		// 'II10'
};

struct IMFSAMPLE
{
	char filename[13];	// Sample filename (12345678.ABC) */
	uint8 unused1[3];
	uint32 length;		// Length (in bytes)
	uint32 loop_start;	// Loop start (in bytes)
	uint32 loop_end;	// Loop end (in bytes)
	uint32 C5Speed;		// Samplerate
	uint8 volume;		// Default volume (0...64)
	uint8 panning;		// Default pan (0...255)
	uint8 unused2[14];
	uint8 flags;		// Sample flags
	uint8 unused3[5];
	uint16 ems;			// Reserved for internal usage
	uint32 dram;		// Reserved for internal usage
	char is10[4];		// 'IS10'
};
#pragma pack(pop)

static const uint8 imfEffects[] =
{
	CMD_NONE,
	CMD_SPEED,			// 0x01 1xx Set Tempo
	CMD_TEMPO,			// 0x02 2xx Set BPM
	CMD_TONEPORTAMENTO, // 0x03 3xx Tone Portamento
	CMD_TONEPORTAVOL,	// 0x04 4xy Tone Portamento + Volume Slide
	CMD_VIBRATO,		// 0x05 5xy Vibrato
	CMD_VIBRATOVOL,		// 0x06 6xy Vibrato + Volume Slide
	CMD_FINEVIBRATO,	// 0x07 7xy Fine Vibrato
	CMD_TREMOLO,		// 0x08 8xy Tremolo
	CMD_ARPEGGIO,		// 0x09 9xy Arpeggio
	CMD_PANNING8,		// 0x0A Axx Set Pan Position
	CMD_PANNINGSLIDE,	// 0x0B Bxy Pan Slide
	CMD_VOLUME,			// 0x0C Cxx Set Volume
	CMD_VOLUMESLIDE,	// 0x0D Dxy Volume Slide
	CMD_VOLUMESLIDE,	// 0x0E Exy Fine Volume Slide
	CMD_S3MCMDEX,		// 0x0F Fxx Set Finetune
	CMD_NOTESLIDEUP,	// 0x10 Gxy Note Slide Up
	CMD_NOTESLIDEDOWN,	// 0x11 Hxy Note Slide Down
	CMD_PORTAMENTOUP,	// 0x12 Ixx Slide Up
	CMD_PORTAMENTODOWN,	// 0x13 Jxx Slide Down
	CMD_PORTAMENTOUP,	// 0x14 Kxx Fine Slide Up
	CMD_PORTAMENTODOWN,	// 0x15 Lxx Fine Slide Down
	CMD_MIDI,			// 0x16 Mxx Set Filter Cutoff - XXX
	CMD_NONE,			// 0x17 Nxy Filter Slide + Resonance - XXX
	CMD_OFFSET,			// 0x18 Oxx Set Sample Offset
	CMD_NONE,			// 0x19 Pxx Set Fine Sample Offset - XXX
	CMD_KEYOFF,			// 0x1A Qxx Key Off
	CMD_RETRIG,			// 0x1B Rxy Retrig
	CMD_TREMOR,			// 0x1C Sxy Tremor
	CMD_POSITIONJUMP,	// 0x1D Txx Position Jump
	CMD_PATTERNBREAK,	// 0x1E Uxx Pattern Break
	CMD_GLOBALVOLUME,	// 0x1F Vxx Set Mastervolume
	CMD_GLOBALVOLSLIDE,	// 0x20 Wxy Mastervolume Slide
	CMD_S3MCMDEX,		// 0x21 Xxx Extended Effect
							// X1x Set Filter
							// X3x Glissando
							// X5x Vibrato Waveform
							// X8x Tremolo Waveform
							// XAx Pattern Loop
							// XBx Pattern Delay
							// XCx Note Cut
							// XDx Note Delay
							// XEx Ignore Envelope
							// XFx Invert Loop
	CMD_NONE,			// 0x22 Yxx Chorus - XXX
	CMD_NONE,			// 0x23 Zxx Reverb - XXX
};

static void ImportIMFEffect(ModCommand *note)
//-------------------------------------------
{
	uint8 n;
	// fix some of them
	switch (note->command)
	{
	case 0xE: // fine volslide
		// hackaround to get almost-right behavior for fine slides (i think!)
		if(note->param == 0)
			/* nothing */;
		else if(note->param == 0xF0)
			note->param = 0xEF;
		else if(note->param == 0x0F)
			note->param = 0xFE;
		else if(note->param & 0xF0)
			note->param |= 0x0F;
		else
			note->param |= 0xF0;
		break;
	case 0xF: // set finetune
		// we don't implement this, but let's at least import the value
		note->param = 0x20 | min(note->param >> 4, 0xf);
		break;
	case 0x14: // fine slide up
	case 0x15: // fine slide down
		// this is about as close as we can do...
		if(note->param >> 4)
			note->param = 0xf0 | min(note->param >> 4, 0xf);
		else
			note->param |= 0xe0;
		break;
	case 0x16: // cutoff
		note->param >>= 1;
		break;
	case 0x1F: // set global volume
		note->param = min(note->param << 1, 0xff);
		break;
	case 0x21:
		n = 0;
		switch (note->param >> 4)
		{
		case 0:
			/* undefined, but since S0x does nothing in IT anyway, we won't care.
			this is here to allow S00 to pick up the previous value (assuming IMF
			even does that -- I haven't actually tried it) */
			break;
		default: // undefined
		case 0x1: // set filter
		case 0xF: // invert loop
			note->command = CMD_NONE;
			break;
		case 0x3: // glissando
			n = 0x20;
			break;
		case 0x5: // vibrato waveform
			n = 0x30;
			break;
		case 0x8: // tremolo waveform
			n = 0x40;
			break;
		case 0xA: // pattern loop
			n = 0xB0;
			break;
		case 0xB: // pattern delay
			n = 0xE0;
			break;
		case 0xC: // note cut
		case 0xD: // note delay
			// no change
			break;
		case 0xE: // ignore envelope
			/* predicament: we can only disable one envelope at a time.
			volume is probably most noticeable, so let's go with that.
			(... actually, orpheus doesn't even seem to implement this at all) */
			note->param = 0x77;
			break;
		case 0x18: // sample offset
			// O00 doesn't pick up the previous value
			if(!note->param)
				note->command = CMD_NONE;
			break;
		}
		if(n)
			note->param = n | (note->param & 0x0F);
		break;
	}
	note->command = (note->command < CountOf(imfEffects)) ? imfEffects[note->command] : CMD_NONE;
	if(note->command == CMD_VOLUME && note->volcmd == VOLCMD_NONE)
	{
		note->volcmd = VOLCMD_VOLUME;
		note->vol = note->param;
		note->command = CMD_NONE;
		note->param = 0;
	}
}

static void LoadIMFEnvelope(InstrumentEnvelope *env, const IMFINSTRUMENT *imfins, const int e)
//--------------------------------------------------------------------------------------------
{
	const int shift = (e == IMF_ENV_VOL) ? 0 : 2;

	env->dwFlags.set(ENV_ENABLED, (imfins->env[e].flags & 1) != 0);
	env->dwFlags.set(ENV_SUSTAIN, (imfins->env[e].flags & 2) != 0);
	env->dwFlags.set(ENV_LOOP, (imfins->env[e].flags & 4) != 0);

	env->nNodes = imfins->env[e].points;
	Limit(env->nNodes, 2u, 16u);
	env->nLoopStart = imfins->env[e].loop_start;
	env->nLoopEnd = imfins->env[e].loop_end;
	env->nSustainStart = env->nSustainEnd = imfins->env[e].sustain;

	uint16 min = 0; // minimum tick value for next node
	for(uint32 n = 0; n < env->nNodes; n++)
	{
		uint16 nTick, nValue;
		nTick = LittleEndianW(imfins->nodes[e][n].tick);
		nValue = LittleEndianW(imfins->nodes[e][n].value) >> shift;
		env->Ticks[n] = (uint16)max(min, nTick);
		env->Values[n] = (uint8)min(nValue, ENVELOPE_MAX);
		min = nTick + 1;
	}
}

bool CSoundFile::ReadIMF(const LPCBYTE lpStream, const DWORD dwMemLength)
//-----------------------------------------------------------------------
{
	DWORD dwMemPos = 0;
	vector<bool> ignoreChannels(32, false); // bit set for each channel that's completely disabled

	ASSERT_CAN_READ(sizeof(IMFHEADER));
	IMFHEADER hdr;
	memcpy(&hdr, lpStream, sizeof(IMFHEADER));
	dwMemPos = sizeof(IMFHEADER);

	hdr.ordnum = LittleEndianW(hdr.ordnum);
	hdr.patnum = LittleEndianW(hdr.patnum);
	hdr.insnum = LittleEndianW(hdr.insnum);
	hdr.flags = LittleEndianW(hdr.flags);

	if(memcmp(hdr.im10, "IM10", 4) != 0)
		return false;

	m_nType = MOD_TYPE_IMF;
	SetModFlag(MSF_COMPATIBLE_PLAY, true);

	// song name
	MemsetZero(m_szNames);
	StringFixer::ReadString<StringFixer::nullTerminated>(m_szNames[0], hdr.title);

	m_SongFlags = (hdr.flags & 1) ? SONG_LINEARSLIDES : SongFlags(0);
	m_nDefaultSpeed = hdr.tempo;
	m_nDefaultTempo = hdr.bpm;
	m_nDefaultGlobalVolume = CLAMP(hdr.master, 0, 64) << 2;
	m_nSamplePreAmp = CLAMP(hdr.amp, 4, 127);

	m_nSamples = 0; // Will be incremented later
	m_nInstruments = 0;

	m_nChannels = 0;
	for(CHANNELINDEX nChn = 0; nChn < 32; nChn++)
	{
		ChnSettings[nChn].nPan = hdr.channels[nChn].panning * 64 / 255;
		ChnSettings[nChn].nPan *= 4;

		StringFixer::ReadString<StringFixer::nullTerminated>(ChnSettings[nChn].szName, hdr.channels[nChn].name);

		// TODO: reverb/chorus?
		switch(hdr.channels[nChn].status)
		{
		case 0: // enabled; don't worry about it
			m_nChannels = nChn + 1;
			break;
		case 1: // mute
			ChnSettings[nChn].dwFlags = CHN_MUTE;
			m_nChannels = nChn + 1;
			break;
		case 2: // disabled
			ChnSettings[nChn].dwFlags = CHN_MUTE;
			ignoreChannels[nChn] = true;
			break;
		default: // uhhhh.... freak out
			//fprintf(stderr, "imf: channel %d has unknown status %d\n", n, hdr.channels[n].status);
			return false;
		}
	}
	if(!m_nChannels) return false;

	//From mikmod: work around an Orpheus bug
	if(hdr.channels[0].status == 0)
	{
		CHANNELINDEX nChn;
		for(nChn = 1; nChn < 16; nChn++)
			if(hdr.channels[nChn].status != 1)
				break;
		if(nChn == 16)
			for(nChn = 1; nChn < 16; nChn++)
				ChnSettings[nChn].dwFlags.reset(CHN_MUTE);
	}

	Order.resize(hdr.ordnum);
	for(ORDERINDEX nOrd = 0; nOrd < hdr.ordnum; nOrd++)
		Order[nOrd] = ((hdr.orderlist[nOrd] == 0xFF) ? Order.GetIgnoreIndex() : (PATTERNINDEX)hdr.orderlist[nOrd]);

	// read patterns
	for(PATTERNINDEX nPat = 0; nPat < hdr.patnum; nPat++)
	{
		uint16 length, nrows;
		uint8 mask, channel;
		int row;
		unsigned int lostfx = 0;
		ModCommand *note, junk_note;

		ASSERT_CAN_READ(4);
		length = LittleEndianW(*((uint16 *)(lpStream + dwMemPos)));
		nrows = LittleEndianW(*((uint16 *)(lpStream + dwMemPos + 2)));
		dwMemPos += 4;

		if(Patterns.Insert(nPat, nrows))
			break;

		row = 0;
		while(row < nrows)
		{
			ASSERT_CAN_READ(1);
			mask = *((uint8 *)(lpStream + dwMemPos));
			dwMemPos += 1;
			if(mask == 0)
			{
				row++;
				continue;
			}

			channel = mask & 0x1F;

			if(ignoreChannels[channel])
			{
				/* should do this better, i.e. not go through the whole process of deciding
				what to do with the effects since they're just being thrown out */
				//printf("disabled channel %d contains data\n", channel + 1);
				note = &junk_note;
			} else
			{
				note = Patterns[nPat].GetpModCommand(row, channel);
			}

			if(mask & 0x20)
			{
				// read note/instrument
				ASSERT_CAN_READ(2);
				note->note = *((BYTE *)(lpStream + dwMemPos));
				note->instr = *((BYTE *)(lpStream + dwMemPos + 1));
				dwMemPos += 2;

				if(note->note == 160)
				{
					note->note = NOTE_KEYOFF; /* ??? */
				} else if(note->note == 255)
				{
					note->note = NOTE_NONE; /* ??? */
				} else
				{
					note->note = (note->note >> 4) * 12 + (note->note & 0x0F) + 12 + 1;
					if(note->note > NOTE_MAX)
					{
						/*printf("%d.%d.%d: funny note 0x%02x\n",
							nPat, row, channel, fp->data[fp->pos - 1]);*/
						note->note = NOTE_NONE;
					}
				}
			}
			if((mask & 0xc0) == 0xC0)
			{
				// read both effects and figure out what to do with them
				ASSERT_CAN_READ(4);
				uint8 e1c = *((uint8 *)(lpStream + dwMemPos));		// Command 1
				uint8 e1d = *((uint8 *)(lpStream + dwMemPos + 1));	// Data 1
				uint8 e2c = *((uint8 *)(lpStream + dwMemPos + 2));	// Command 2
				uint8 e2d = *((uint8 *)(lpStream + dwMemPos + 3));	// Data 2
				dwMemPos += 4;

				if(e1c == 0x0C)
				{
					note->vol = min(e1d, 0x40);
					note->volcmd = VOLCMD_VOLUME;
					note->command = e2c;
					note->param = e2d;
				} else if(e2c == 0x0C)
				{
					note->vol = min(e2d, 0x40);
					note->volcmd = VOLCMD_VOLUME;
					note->command = e1c;
					note->param = e1d;
				} else if(e1c == 0x0A)
				{
					note->vol = e1d * 64 / 255;
					note->volcmd = VOLCMD_PANNING;
					note->command = e2c;
					note->param = e2d;
				} else if(e2c == 0x0A)
				{
					note->vol = e2d * 64 / 255;
					note->volcmd = VOLCMD_PANNING;
					note->command = e1c;
					note->param = e1d;
				} else
				{
					/* check if one of the effects is a 'global' effect
					-- if so, put it in some unused channel instead.
					otherwise pick the most important effect. */
					lostfx++;
					note->command = e2c;
					note->param = e2d;
				}
			} else if(mask & 0xC0)
			{
				// there's one effect, just stick it in the effect column
				ASSERT_CAN_READ(2);
				note->command = *((BYTE *)(lpStream + dwMemPos));
				note->param = *((BYTE *)(lpStream + dwMemPos + 1));
				dwMemPos += 2;
			}
			if(note->command)
				ImportIMFEffect(note);
		}
	}

	SAMPLEINDEX firstsample = 1; // first sample index of the current instrument

	// read instruments
	for(INSTRUMENTINDEX nIns = 0; nIns < hdr.insnum; nIns++)
	{
		IMFINSTRUMENT imfins;
		ModInstrument *pIns;
		ASSERT_CAN_READ(sizeof(IMFINSTRUMENT));
		memcpy(&imfins, lpStream + dwMemPos, sizeof(IMFINSTRUMENT));
		dwMemPos += sizeof(IMFINSTRUMENT);
		m_nInstruments++;

		imfins.smpnum = LittleEndianW(imfins.smpnum);
		imfins.fadeout = LittleEndianW(imfins.fadeout);

		// Orpheus does not check this!
		//if(memcmp(imfins.ii10, "II10", 4) != 0)
		//	return false;

		pIns = AllocateInstrument(nIns + 1);
		if(pIns == nullptr)
		{
			continue;
		}

		StringFixer::ReadString<StringFixer::nullTerminated>(pIns->name, imfins.name);

		if(imfins.smpnum)
		{
			STATIC_ASSERT(CountOf(pIns->Keyboard) >= CountOf(imfins.map));
			for(size_t cNote = 0; cNote < CountOf(imfins.map); cNote++)
			{
				pIns->Keyboard[cNote] = firstsample + imfins.map[cNote];
			}
		}

		pIns->nFadeOut = imfins.fadeout;

		LoadIMFEnvelope(&pIns->VolEnv, &imfins, IMF_ENV_VOL);
		LoadIMFEnvelope(&pIns->PanEnv, &imfins, IMF_ENV_PAN);
		LoadIMFEnvelope(&pIns->PitchEnv, &imfins, IMF_ENV_FILTER);
		if((pIns->PitchEnv.dwFlags & ENV_ENABLED) != 0)
			pIns->PitchEnv.dwFlags |= ENV_FILTER;

		// hack to get === to stop notes (from modplug's xm loader)
		if(!pIns->VolEnv.dwFlags[ENV_ENABLED] && !pIns->nFadeOut)
			pIns->nFadeOut = 8192;

		// read this instrument's samples
		for(SAMPLEINDEX nSmp = 0; nSmp < imfins.smpnum; nSmp++)
		{
			ASSERT_CAN_READ(sizeof(IMFSAMPLE));
			IMFSAMPLE imfsmp;
			memcpy(&imfsmp, lpStream + dwMemPos, sizeof(IMFSAMPLE));
			dwMemPos += sizeof(IMFSAMPLE);
			m_nSamples++;

			if(memcmp(imfsmp.is10, "IS10", 4) != 0)
				return false;

			ModSample &sample = Samples[firstsample + nSmp];

			sample.Initialize();
			StringFixer::ReadString<StringFixer::nullTerminated>(sample.filename, imfsmp.filename);
			strcpy(m_szNames[m_nSamples], sample.filename);

			uint32 byteLen = sample.nLength = LittleEndian(imfsmp.length);
			sample.nLoopStart = LittleEndian(imfsmp.loop_start);
			sample.nLoopEnd = LittleEndian(imfsmp.loop_end);
			sample.nC5Speed = LittleEndian(imfsmp.C5Speed);
			sample.nVolume = imfsmp.volume * 4;
			sample.nPan = imfsmp.panning;
			if(imfsmp.flags & 1)
				sample.uFlags |= CHN_LOOP;
			if(imfsmp.flags & 2)
				sample.uFlags |= CHN_PINGPONGLOOP;
			if(imfsmp.flags & 4)
			{
				sample.uFlags |= CHN_16BIT;
				sample.nLength /= 2;
				sample.nLoopStart /= 2;
				sample.nLoopEnd /= 2;
			}
			if(imfsmp.flags & 8)
				sample.uFlags |= CHN_PANNING;

			if(byteLen)
			{
				ASSERT_CAN_READ(byteLen);

				SampleIO(
					(imfsmp.flags & 4) ? SampleIO::_16bit : SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::signedPCM)
					.ReadSample(sample, reinterpret_cast<LPCSTR>(lpStream + dwMemPos), byteLen);
			}

			dwMemPos += byteLen;
		}
		firstsample += imfins.smpnum;
	}

	return true;
}