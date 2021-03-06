/*
 * Load_okt.cpp
 * ------------
 * Purpose: OKT (Oktalyzer) module loader
 * Notes  : (currently none)
 * Authors: Storlek (Original author - http://schismtracker.org/ - code ported with permission)
 *			Johannes Schultz (OpenMPT Port, tweaks)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#pragma pack(push, 1)

struct OktIffChunk
{
	// IFF chunk names
	enum ChunkIdentifiers
	{
		idCMOD	= 0x434D4F44,
		idSAMP	= 0x53414D50,
		idSPEE	= 0x53504545,
		idSLEN	= 0x534C454E,
		idPLEN	= 0x504C454E,
		idPATT	= 0x50415454,
		idPBOD	= 0x50424F44,
		idSBOD	= 0x53424F44,
	};

	uint32 signature;	// IFF chunk name
	uint32 chunksize;	// chunk size without header

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(signature);
		SwapBytesBE(chunksize);
	}
};

struct OktSample
{
	char   name[20];
	uint32 length;		// length in bytes
	uint16 loopStart;	// *2 for real value
	uint16 loopLength;	// dito
	uint16 volume;		// default volume
	uint16 type;		// 7-/8-bit sample

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(length);
		SwapBytesBE(loopStart);
		SwapBytesBE(loopLength);
		SwapBytesBE(volume);
		SwapBytesBE(type);
	}
};

STATIC_ASSERT(sizeof(OktSample) == 32);

#pragma pack(pop)

// Parse the sample header block
void ReadOKTSamples(FileReader &chunk, vector<bool> &sample7bit, CSoundFile *pSndFile)
//------------------------------------------------------------------------------------
{
	pSndFile->m_nSamples = min((SAMPLEINDEX)(chunk.BytesLeft() / sizeof(OktSample)), MAX_SAMPLES - 1);	// typically 36
	sample7bit.resize(pSndFile->GetNumSamples());

	for(SAMPLEINDEX nSmp = 1; nSmp <= pSndFile->GetNumSamples(); nSmp++)
	{
		ModSample &mptSmp = pSndFile->GetSample(nSmp);
		OktSample oktSmp;
		chunk.ReadConvertEndianness(oktSmp);

		oktSmp.length = oktSmp.length;
		oktSmp.loopStart = oktSmp.loopStart * 2;
		oktSmp.loopLength = oktSmp.loopLength * 2;
		oktSmp.volume = oktSmp.volume;
		oktSmp.type = oktSmp.type;

		mptSmp.Initialize();
		StringFixer::ReadString<StringFixer::maybeNullTerminated>(pSndFile->m_szNames[nSmp], oktSmp.name);

		mptSmp.nC5Speed = 8287;
		mptSmp.nGlobalVol = 64;
		mptSmp.nVolume = min(oktSmp.volume, 64) * 4;
		mptSmp.nLength = oktSmp.length & ~1;	// round down
		// parse loops
		if (oktSmp.loopLength > 2 && static_cast<SmpLength>(oktSmp.loopStart) + static_cast<SmpLength>(oktSmp.loopLength) <= mptSmp.nLength)
		{
			mptSmp.nSustainStart = oktSmp.loopStart;
			mptSmp.nSustainEnd = oktSmp.loopStart + oktSmp.loopLength;
			if (mptSmp.nSustainStart < mptSmp.nLength && mptSmp.nSustainEnd <= mptSmp.nLength)
				mptSmp.uFlags |= CHN_SUSTAINLOOP;
			else
				mptSmp.nSustainStart = mptSmp.nSustainEnd = 0;
		}
		sample7bit[nSmp - 1] = (oktSmp.type == 0 || oktSmp.type == 2);
	}
}


// Parse a pattern block
void ReadOKTPattern(FileReader &chunk, PATTERNINDEX nPat, CSoundFile *pSndFile)
//-----------------------------------------------------------------------------
{
	if(!chunk.CanRead(2))
	{
		return;
	}

	ROWINDEX rows = Clamp(static_cast<ROWINDEX>(chunk.ReadUint16BE()), ROWINDEX(1), MAX_PATTERN_ROWS);

	if(pSndFile->Patterns.Insert(nPat, rows))
	{
		return;
	}

	const CHANNELINDEX chns = pSndFile->GetNumChannels();

	for(ROWINDEX row = 0; row < rows; row++)
	{
		ModCommand *m = pSndFile->Patterns[nPat].GetRow(row);
		for(CHANNELINDEX chn = 0; chn < chns; chn++, m++)
		{
			uint8 note = chunk.ReadUint8();
			uint8 instr = chunk.ReadUint8();
			uint8 effect = chunk.ReadUint8();
			m->param = chunk.ReadUint8();

			if(note > 0 && note <= 36)
			{
				m->note = note + 48;
				m->instr = instr + 1;
			} else
			{
				m->instr = 0;
			}

			switch(effect)
			{
			case 0:	// Nothing
				m->param = 0;
				break;

			case 1: // 1 Portamento Down (Period)
				m->command = CMD_PORTAMENTODOWN;
				m->param &= 0x0F;
				break;
			case 2: // 2 Portamento Up (Period)
				m->command = CMD_PORTAMENTOUP;
				m->param &= 0x0F;
				break;

#if 0
			/* these aren't like Jxx: "down" means to *subtract* the offset from the note.
			For now I'm going to leave these unimplemented. */
			case 10: // A Arpeggio 1 (down, orig, up)
			case 11: // B Arpeggio 2 (orig, up, orig, down)
				if (m->param)
					m->command = CMD_ARPEGGIO;
				break;
#endif
			// This one is close enough to "standard" arpeggio -- I think!
			case 12: // C Arpeggio 3 (up, up, orig)
				if (m->param)
					m->command = CMD_ARPEGGIO;
				break;

			case 13: // D Slide Down (Notes)
				if (m->param)
				{
					m->command = CMD_NOTESLIDEDOWN;
					m->param = 0x10 | min(0x0F, m->param);
				}
				break;
			case 30: // U Slide Up (Notes)
				if (m->param)
				{
					m->command = CMD_NOTESLIDEUP;
					m->param = 0x10 | min(0x0F, m->param);
				}
				break;
			/* We don't have fine note slide, but this is supposed to happen once
			per row. Sliding every 5 (non-note) ticks kind of works (at least at
			speed 6), but implementing fine slides would of course be better. */
			case 21: // L Slide Down Once (Notes)
				if (m->param)
				{
					m->command = CMD_NOTESLIDEDOWN;
					m->param = 0x50 | min(0x0F, m->param);
				}
				break;
			case 17: // H Slide Up Once (Notes)
				if (m->param)
				{
					m->command = CMD_NOTESLIDEUP;
					m->param = 0x50 | min(0x0F, m->param);
				}
				break;

			case 15: // F Set Filter <>00:ON
				// Not implemented, but let's import it anyway...
				m->command = CMD_MODCMDEX;
				m->param = !!m->param;
				break;

			case 25: // P Pos Jump
				m->command = CMD_POSITIONJUMP;
				break;

			case 27: // R Release sample (apparently not listed in the help!)
				m->Clear();
				m->note = NOTE_KEYOFF;
				break;

			case 28: // S Speed
				m->command = CMD_SPEED; // or tempo?
				break;

			case 31: // V Volume
				m->command = CMD_VOLUMESLIDE;
				switch (m->param >> 4)
				{
				case 4:
					if (m->param != 0x40)
					{
						m->param &= 0x0F; // D0x
						break;
					}
					// 0x40 is set volume -- fall through
				case 0: case 1: case 2: case 3:
					m->volcmd = VOLCMD_VOLUME;
					m->vol = m->param;
					m->command = CMD_NONE;
					m->param = 0;
					break;
				case 5:
					m->param = (m->param & 0x0F) << 4; // Dx0
					break;
				case 6:
					m->param = 0xF0 | min(m->param & 0x0F, 0x0E); // DFx
					break;
				case 7:
					m->param = (min(m->param & 0x0F, 0x0E) << 4) | 0x0F; // DxF
					break;
				default:
					// Junk.
					m->command = m->param = 0;
					break;
				}
				break;

#if 0
			case 24: // O Old Volume (???)
				m->command = CMD_VOLUMESLIDE;
				m->param = 0;
				break;
#endif

			default:
				m->command = m->param = 0;
				//ASSERT(false);
				break;
			}
		}
	}
}


bool CSoundFile::ReadOKT(FileReader &file)
//----------------------------------------
{
	file.Rewind();
	if(!file.ReadMagic("OKTASONG"))
	{
		return false;
	}

	// prepare some arrays to store offsets etc.
	vector<FileReader> patternChunks;
	vector<FileReader> sampleChunks;
	vector<bool> sample7bit;	// 7-/8-bit sample
	ORDERINDEX nOrders = 0;

	MemsetZero(m_szNames);
	m_nChannels = 0;
	m_nSamples = 0;

	// Go through IFF chunks...
	while(file.BytesLeft())
	{
		OktIffChunk iffHead;
		if(!file.ReadConvertEndianness(iffHead))
		{
			break;
		}

		FileReader chunk = file.GetChunk(iffHead.chunksize);
		if(!chunk.IsValid())
		{
			break;
		}

		switch(iffHead.signature)
		{
		case OktIffChunk::idCMOD:
			// read that weird channel setup table
			if(m_nChannels > 0 || chunk.GetLength() < 8)
			{
				break;
			}

			for(CHANNELINDEX nChn = 0; nChn < 4; nChn++)
			{
				uint8 ch1 = chunk.ReadUint8(), ch2 = chunk.ReadUint8();
				if(ch1 || ch2)
				{
					ChnSettings[m_nChannels++].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 0xC0 : 0x40;
				}
				ChnSettings[m_nChannels++].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 0xC0 : 0x40;
			}
			break;

		case OktIffChunk::idSAMP:
			// convert sample headers
			if(m_nSamples > 0)
			{
				break;
			}
			ReadOKTSamples(chunk, sample7bit, this);
			break;

		case OktIffChunk::idSPEE:
			// read default speed
			if(chunk.GetLength() >= 2)
			{
				m_nDefaultSpeed = Clamp(chunk.ReadUint16BE(), uint16(1), uint16(255));
			}
			break;

		case OktIffChunk::idSLEN:
			// number of patterns, we don't need this.
			break;

		case OktIffChunk::idPLEN:
			// read number of valid orders
			if(chunk.GetLength() >= 2)
			{
				nOrders = chunk.ReadUint16BE();
			}
			break;

		case OktIffChunk::idPATT:
			// read the orderlist
			Order.ReadAsByte(chunk, chunk.GetLength());
			break;

		case OktIffChunk::idPBOD:
			// don't read patterns for now, as the number of channels might be unknown at this point.
			if(patternChunks.size() < MAX_PATTERNS)
			{
				patternChunks.push_back(chunk);
			}
			break;

		case OktIffChunk::idSBOD:
			// sample data - same as with patterns, as we need to know the sample format / length
			if(sampleChunks.size() < MAX_SAMPLES - 1 && chunk.GetLength() > 0)
			{
				sampleChunks.push_back(chunk);
			}
			break;
		}
	}

	// If there wasn't even a CMOD chunk, we can't really load this.
	if(m_nChannels == 0)
		return false;

	m_nDefaultTempo = 125;
	m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	m_nSamplePreAmp = m_nVSTiVolume = 48;
	m_nType = MOD_TYPE_OKT;
	m_nMinPeriod = 0x71 * 4;
	m_nMaxPeriod = 0x358 * 4;

	// Fix orderlist
	for(ORDERINDEX nOrd = nOrders; nOrd < Order.GetLengthTailTrimmed(); nOrd++)
	{
		Order[nOrd] = Order.GetInvalidPatIndex();
	}

	// Read patterns
	for(PATTERNINDEX nPat = 0; nPat < patternChunks.size(); nPat++)
	{
		if(patternChunks[nPat].GetLength() > 0)
			ReadOKTPattern(patternChunks[nPat], nPat, this);
		else
			Patterns.Insert(nPat, 64);	// invent empty pattern
	}

	// Read samples
	size_t nFileSmp = 0;
	for(SAMPLEINDEX nSmp = 1; nSmp < m_nSamples; nSmp++)
	{
		if(nFileSmp >= sampleChunks.size())
			break;

		ModSample &mptSample = Samples[nSmp];
		if(mptSample.nLength == 0)
			continue;

		// weird stuff?
		mptSample.nLength = min(mptSample.nLength, sampleChunks[nFileSmp].GetLength());

		SampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::bigEndian,
			sample7bit[nSmp - 1] ? SampleIO::PCM7to8 : SampleIO::signedPCM)
			.ReadSample(mptSample, sampleChunks[nFileSmp]);

		nFileSmp++;
	}

	SetModFlag(MSF_COMPATIBLE_PLAY, true);

	return true;
}