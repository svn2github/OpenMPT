/*
 * Load_s3m.cpp
 * ------------
 * Purpose: S3M (ScreamTracker 3) module loader / saver
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "../mptrack/version.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/moddoc.h"	// Logging
#endif // MODPLUG_TRACKER


void CSoundFile::S3MConvert(ModCommand &m, bool fromIT) const
//--------------------------------------------------------
{
	switch(m.command | 0x40)
	{
	case 'A':	m.command = CMD_SPEED; break;
	case 'B':	m.command = CMD_POSITIONJUMP; break;
	case 'C':	m.command = CMD_PATTERNBREAK; if (!fromIT) m.param = (m.param >> 4) * 10 + (m.param & 0x0F); break;
	case 'D':	m.command = CMD_VOLUMESLIDE; break;
	case 'E':	m.command = CMD_PORTAMENTODOWN; break;
	case 'F':	m.command = CMD_PORTAMENTOUP; break;
	case 'G':	m.command = CMD_TONEPORTAMENTO; break;
	case 'H':	m.command = CMD_VIBRATO; break;
	case 'I':	m.command = CMD_TREMOR; break;
	case 'J':	m.command = CMD_ARPEGGIO; break;
	case 'K':	m.command = CMD_VIBRATOVOL; break;
	case 'L':	m.command = CMD_TONEPORTAVOL; break;
	case 'M':	m.command = CMD_CHANNELVOLUME; break;
	case 'N':	m.command = CMD_CHANNELVOLSLIDE; break;
	case 'O':	m.command = CMD_OFFSET; break;
	case 'P':	m.command = CMD_PANNINGSLIDE; break;
	case 'Q':	m.command = CMD_RETRIG; break;
	case 'R':	m.command = CMD_TREMOLO; break;
	case 'S':	m.command = CMD_S3MCMDEX; break;
	case 'T':	m.command = CMD_TEMPO; break;
	case 'U':	m.command = CMD_FINEVIBRATO; break;
	case 'V':	m.command = CMD_GLOBALVOLUME; break;
	case 'W':	m.command = CMD_GLOBALVOLSLIDE; break;
	case 'X':	m.command = CMD_PANNING8; break;
	case 'Y':	m.command = CMD_PANBRELLO; break;
	case 'Z':	m.command = CMD_MIDI; break;
	case '\\':	m.command = fromIT ? CMD_SMOOTHMIDI : CMD_MIDI; break; //rewbs.smoothVST
	// Chars under 0x40 don't save properly, so map : to ] and # to [.
	case ']':	m.command = CMD_DELAYCUT; break;
	case '[':	m.command = CMD_XPARAM; break;
	default:	m.command = CMD_NONE;
	}
}


void CSoundFile::S3MSaveConvert(uint8 &command, uint8 &param, bool toIT, bool compatibilityExport) const
//------------------------------------------------------------------------------------------------------
{
	switch(command)
	{
	case CMD_SPEED:				command = 'A'; break;
	case CMD_POSITIONJUMP:		command = 'B'; break;
	case CMD_PATTERNBREAK:		command = 'C'; if(!toIT) param = ((param / 10) << 4) + (param % 10); break;
	case CMD_VOLUMESLIDE:		command = 'D'; break;
	case CMD_PORTAMENTODOWN:	command = 'E'; if (param >= 0xE0 && (GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_PORTAMENTOUP:		command = 'F'; if (param >= 0xE0 && (GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_TONEPORTAMENTO:	command = 'G'; break;
	case CMD_VIBRATO:			command = 'H'; break;
	case CMD_TREMOR:			command = 'I'; break;
	case CMD_ARPEGGIO:			command = 'J'; break;
	case CMD_VIBRATOVOL:		command = 'K'; break;
	case CMD_TONEPORTAVOL:		command = 'L'; break;
	case CMD_CHANNELVOLUME:		command = 'M'; break;
	case CMD_CHANNELVOLSLIDE:	command = 'N'; break;
	case CMD_OFFSET:			command = 'O'; break;
	case CMD_PANNINGSLIDE:		command = 'P'; break;
	case CMD_RETRIG:			command = 'Q'; break;
	case CMD_TREMOLO:			command = 'R'; break;
	case CMD_S3MCMDEX:			command = 'S'; break;
	case CMD_TEMPO:				command = 'T'; break;
	case CMD_FINEVIBRATO:		command = 'U'; break;
	case CMD_GLOBALVOLUME:		command = 'V'; break;
	case CMD_GLOBALVOLSLIDE:	command = 'W'; break;
	case CMD_PANNING8:			
		command = 'X';
		if(toIT && !(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM | MOD_TYPE_MOD)))
		{
			if (param == 0xA4) { command = 'S'; param = 0x91; }	else
			if (param <= 0x80) { param <<= 1; if (param > 255) param = 255; } else
			command = 0;
		} else if (!toIT && (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM | MOD_TYPE_MOD)))
		{
			param >>= 1;
		}
		break;
	case CMD_PANBRELLO:			command = 'Y'; break;
	case CMD_MIDI:				command = 'Z'; break;
	case CMD_SMOOTHMIDI:  //rewbs.smoothVST
		if(compatibilityExport || !toIT)
			command = 'Z';
		else
			command = '\\';
		break;
	case CMD_XFINEPORTAUPDOWN:
		if(param & 0x0F) switch(param & 0xF0)
		{
		case 0x10:	command = 'F'; param = (param & 0x0F) | 0xE0; break;
		case 0x20:	command = 'E'; param = (param & 0x0F) | 0xE0; break;
		case 0x90:	command = 'S'; break;
		default:	command = 0;
		} else command = 0;
		break;
	case CMD_MODCMDEX:
		{
			ModCommand m;
			m.command = CMD_MODCMDEX;
			m.param = param;
			m.ExtendedMODtoS3MEffect();
			command = m.command;
			param = m.param;
			S3MSaveConvert(command, param, toIT, compatibilityExport);
		}
		return;
	// Chars under 0x40 don't save properly, so map : to ] and # to [.
	case CMD_DELAYCUT: 
		if(compatibilityExport || !toIT)
			command = 0;
		else
			command = ']';
		break;
	case CMD_XPARAM:
		if(compatibilityExport || !toIT)
			command = 0;
		else
			command = '[';
		break;
	default:
		command = 0;
	}
	if(command == 0)
	{
		param = 0;
	}

	command &= ~0x40;
}


#pragma pack(push, 1)

// S3M File Header
struct S3MFileHeader
{
	// Magic Bytes
	enum S3MMagic
	{
		idSCRM				= 0x4D524353,
		idEOF				= 0x1A,
		idS3MType			= 0x10,
		idPanning			= 0xFC,
	};

	// Tracker Versions in the cwtv field
	enum S3MTrackerVersions
	{
		trackerMask			= 0xF000,
		versionMask			= 0x0FFF,

		trkScreamTracker	= 0x1000,
		trkImagoOrpheus		= 0x2000,
		trkImpulseTracker	= 0x3000,
		trkSchismTracker	= 0x4000,
		trkOpenMPT			= 0x5000,

		trkST3_20			= 0x1320,
		trkIT2_14			= 0x3214,
	};

	// Flags
	enum S3MHeaderFlags
	{
		zeroVolOptim		= 0x08,	// Volume 0 optimisations
		amigaLimits			= 0x10,	// Enforce Amiga limits
		fastVolumeSlides	= 0x40,	// Fast volume slides (like in ST3.00)
	};

	// S3M Format Versions
	enum S3MFormatVersion
	{
		oldVersion			= 0x01,	// Old Version, signed samples
		newVersion			= 0x02,	// New Version, unsigned samples
	};

	char   name[28];		// Song Title
	uint8  dosEof;			// Supposed to be 0x1A, but even ST3 seems to ignore this sometimes (see STRSHINE.S3M by Purple Motion)
	uint8  fileType;		// File Type, 0x10 = ST3 module
	char   reserved1[2];	// Reserved
	uint16 ordNum;			// Number of order items
	uint16 smpNum;			// Number of sample parapointers
	uint16 patNum;			// Number of pattern parapointers
	uint16 flags;			// Flags, see S3MHeaderFlags
	uint16 cwtv;			// "Made With" Tracker ID, see S3MTrackerVersions
	uint16 formatVersion;	// Format Version, see S3MFormatVersion
	uint32 magic;			// "SCRM" magic bytes
	uint8  globalVol;		// Default Global Volume (0...64)
	uint8  speed;			// Default Speed (1...254)
	uint8  tempo;			// Default Tempo (33...255)
	uint8  masterVolume;	// Sample Volume (0...127, stereo if high bit is set)
	uint8  ultraClicks;		// Number of channels used for ultra click removal
	uint8  usePanningTable;	// 0xFC => read extended panning table
	char   reserved2[8];	// More reserved bytes
	uint16 special;			// Pointer to special custom data (unused)
	uint8  channels[32];	// Channel setup

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(ordNum);
		SwapBytesLE(smpNum);
		SwapBytesLE(patNum);
		SwapBytesLE(flags);
		SwapBytesLE(cwtv);
		SwapBytesLE(formatVersion);
		SwapBytesLE(magic);
	}
};

STATIC_ASSERT(sizeof(S3MFileHeader) == 96);


// S3M Sample Header
struct S3MSampleHeader
{
	enum SampleMagic
	{
		idSCRS		= 0x53524353,
	};

	enum SampleType
	{
		typeNone	= 0,
		typePCM		= 1,
		typeAdMel	= 2,
	};

	enum SampleFlags
	{
		smpLoop		= 0x01,
		smpStereo	= 0x02,
		smp16Bit	= 0x04,
	};

	enum SamplePacking
	{
		pUnpacked	= 0x00,	// PCM
		pDP30ADPCM	= 0x01,	// Unused packing type
		pADPCM		= 0x04,	// MODPlugin ADPCM :(
	};

	uint8  sampleType;			// Sample type, see SampleType
	char   filename[12];		// Sample filename
	uint8  dataPointer[3];		// Pointer to sample data (divided by 16)
	uint32 length;				// Sample length, in samples
	uint32 loopStart;			// Loop start, in samples
	uint32 loopEnd;				// Loop end, in samples
	uint8  defaultVolume;		// Default volume (0...64)
	char   reserved1;			// Reserved
	uint8  pack;				// Packing algorithm, SamplePacking
	uint8  flags;				// Sample flags
	uint32 c5speed;				// Middle-C frequency
	char   reserved2[12];		// Reserved + Internal ST3 stuff
	char   name[28];			// Sample name
	uint32 magic;				// "SCRS" magic bytes ("SCRI" for Adlib instruments)

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(length);
		SwapBytesLE(loopStart);
		SwapBytesLE(loopEnd);
		SwapBytesLE(c5speed);
		SwapBytesLE(magic);
	}

	// Convert an S3M sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		StringFixer::ReadString<StringFixer::maybeNullTerminated>(mptSmp.filename, filename);

		if((sampleType == typePCM || sampleType == typeNone) && magic == idSCRS)
		{
			// Sample Length and Loops
			if(sampleType == typePCM)
			{
				mptSmp.nLength = min(length, MAX_SAMPLE_LENGTH);
				mptSmp.nLoopStart = min(loopStart, mptSmp.nLength - 1);
				mptSmp.nLoopEnd = min(loopEnd, mptSmp.nLength);
				mptSmp.uFlags = (flags & smpLoop) ? CHN_LOOP : 0;
			} else
			{
				mptSmp.nLength = 0;
				mptSmp.nLoopStart = mptSmp.nLoopEnd = 0;
				mptSmp.uFlags = 0;
			}

			if(mptSmp.nLoopEnd < 2 || mptSmp.nLoopStart >= mptSmp.nLoopEnd || mptSmp.nLoopEnd - mptSmp.nLoopStart < 1)
			{
				mptSmp.nLoopStart = mptSmp.nLoopEnd = 0;
				mptSmp.uFlags = 0;
			}

			// Volume / Panning
			mptSmp.nVolume = min(defaultVolume, 64) * 4;
			mptSmp.nGlobalVol = 64;
			mptSmp.nPan = 128;

			// C-5 frequency
			mptSmp.nC5Speed = c5speed;
			if(mptSmp.nC5Speed == 0)
			{
				mptSmp.nC5Speed = 8363;
			} else if(mptSmp.nC5Speed < 1024)
			{
				mptSmp.nC5Speed = 1024;
			}
		}
	}

	// Convert OpenMPT's internal sample header to an S3M sample header.
	SmpLength ConvertToS3M(const ModSample &mptSmp)
	{
		SmpLength smpLength = 0;
		StringFixer::WriteString<StringFixer::maybeNullTerminated>(filename, mptSmp.filename);

		if(mptSmp.pSample != nullptr)
		{
			sampleType = typePCM;
			length = static_cast<uint32>(min(mptSmp.nLength, uint32_max));
			loopStart = static_cast<uint32>(min(mptSmp.nLoopStart, uint32_max));
			loopEnd = static_cast<uint32>(min(mptSmp.nLoopEnd, uint32_max));

			smpLength = length;

			flags = (mptSmp.uFlags & CHN_LOOP) ? smpLoop : 0;
			if(mptSmp.uFlags & CHN_16BIT)
			{
				flags |= smp16Bit;
			}
			if(mptSmp.uFlags & CHN_STEREO)
			{
				flags |= smpStereo;
			}
		} else
		{
			sampleType = typeNone;
		}

		defaultVolume = static_cast<uint8>(min(mptSmp.nVolume / 4, 64));
		if(mptSmp.nC5Speed != 0)
		{
			c5speed = mptSmp.nC5Speed;
		} else
		{
			c5speed = CSoundFile::TransposeToFrequency(mptSmp.RelativeTone, mptSmp.nFineTune);
		}
		magic = idSCRS;

		return smpLength;
	}

};


// Pattern decoding flags
enum S3MPattern
{
	s3mEndOfRow		= 0x00,
	s3mChannelMask		= 0x1F,
	s3mNotePresent		= 0x20,
	s3mVolumePresent	= 0x40,
	s3mEffectPresent	= 0x80,
	s3mAnyPresent		= 0xE0,

	s3mNoteOff			= 0xFE,
	s3mNoteNone			= 0xFF,
};

STATIC_ASSERT(sizeof(S3MSampleHeader) == 80);

#pragma pack(pop)


// Functor for fixing PixPlay 4-Bit Zxx panning commands
struct FixPixPlayPanning
//======================
{
	void operator()(ModCommand& m)
	{
		if(m.command == CMD_MIDI)
		{
			m.command = CMD_S3MCMDEX;
			m.param |= 0x80;
		}
	}
};


bool CSoundFile::ReadS3M(FileReader &file)
//----------------------------------------
{
	file.Rewind();

	S3MFileHeader fileHeader;
	if(!file.ReadConvertEndianness(fileHeader) || !file.CanRead(fileHeader.ordNum + (fileHeader.smpNum + fileHeader.patNum) * 2))
	{
		return false;
	}

	// Is it a valid S3M file?
	if(fileHeader.magic != S3MFileHeader::idSCRM
		|| fileHeader.fileType != S3MFileHeader::idS3MType
		|| (fileHeader.formatVersion != S3MFileHeader::oldVersion && fileHeader.formatVersion != S3MFileHeader::newVersion))
	{
		return false;
	}
	
	// ST3 ignored Zxx commands, so if we find that a file was made with ST3, we should erase all MIDI macros.
	bool keepMidiMacros = false;

	if((fileHeader.cwtv & S3MFileHeader::trackerMask) == S3MFileHeader::trkOpenMPT)
	{
		// OpenMPT Version number (Major.Minor)
		m_dwLastSavedWithVersion = (fileHeader.cwtv & S3MFileHeader::versionMask) << 16;
	} else if(fileHeader.cwtv  == S3MFileHeader::trkST3_20 && fileHeader.special == 0 && (fileHeader.ordNum & 0x0F) == 0 && fileHeader.ultraClicks == 0 && (fileHeader.flags & ~0x50) == 0)
	{
		// MPT 1.16 and older versions of OpenMPT - Simply keep default (filter) MIDI macros
		m_dwLastSavedWithVersion = MAKE_VERSION_NUMERIC(1, 16, 00, 00);
		keepMidiMacros = true;
	}

	if((fileHeader.cwtv & S3MFileHeader::trackerMask) > S3MFileHeader::trkScreamTracker)
	{
		// 2xyy - Imago Orpheus, 3xyy - IT, 4xyy - Schism, 5xyy - OpenMPT
		if((fileHeader.cwtv & S3MFileHeader::trackerMask) != S3MFileHeader::trkImpulseTracker || fileHeader.cwtv >= S3MFileHeader::trkIT2_14)
		{
			// Keep MIDI macros if this is not an old IT version (BABYLON.S3M by Necros has Zxx commands and was saved with IT 2.05)
			keepMidiMacros = true;
		}
	}

	m_MidiCfg.Reset();
	if(!keepMidiMacros)
	{
		// Remove macros so they don't interfere with tunes made in trackers that don't support Zxx
		MemsetZero(m_MidiCfg.szMidiSFXExt);
		MemsetZero(m_MidiCfg.szMidiZXXExt);
	}

	m_nType = MOD_TYPE_S3M;
	StringFixer::ReadString<StringFixer::nullTerminated>(m_szNames[0], fileHeader.name);

	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;
	m_dwSongFlags = (fileHeader.flags & S3MFileHeader::amigaLimits) ? SONG_AMIGALIMITS : 0;

	if(fileHeader.cwtv < S3MFileHeader::trkST3_20 || (fileHeader.flags & S3MFileHeader::fastVolumeSlides) != 0)
	{
		m_dwSongFlags |= SONG_FASTVOLSLIDES;
	}

	// Speed
	m_nDefaultSpeed = fileHeader.speed;
	if(m_nDefaultSpeed == 0 || m_nDefaultSpeed == 255)
	{
		// Even though ST3 accepts the command AFF as expected, it mysteriously fails to load a default speed of 255...
		m_nDefaultSpeed = 6;
	}

	// Tempo
	m_nDefaultTempo = fileHeader.tempo;
	if(m_nDefaultTempo < 33)
	{
		// ST3 also fails to load an otherwise valid default tempo of 32...
		m_nDefaultTempo = 125;
	}

	// Global Volume
	m_nDefaultGlobalVolume = min(fileHeader.globalVol, 64) * 4;
	// The following check is probably not very reliable, but it fixes a few tunes, f.e.
	// DARKNESS.S3M by Purple Motion (ST 3.00) and "Image of Variance" by C.C.Catch (ST 3.01):
	if(m_nDefaultGlobalVolume == 0 && fileHeader.cwtv < S3MFileHeader::trkST3_20)
	{
		m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	}

	// Bit 8 = Stereo (we always use stereo)
	m_nSamplePreAmp = Clamp(fileHeader.masterVolume & 0x7F, 0x10, 0x7F);

	// Channel setup
	m_nChannels = 4;
	for(CHANNELINDEX i = 0; i < 32; i++)
	{
		ChnSettings[i].nVolume = 64;

		if(fileHeader.channels[i] == 0xFF)
		{
			ChnSettings[i].nPan = 128;
			ChnSettings[i].dwFlags = CHN_MUTE;
		} else
		{
			m_nChannels = i + 1;
			ChnSettings[i].nPan = (fileHeader.channels[i] & 8) ? 192 : 64;	// 200 : 56
			if(fileHeader.channels[i] & 0x80)
			{
				ChnSettings[i].dwFlags = CHN_MUTE;
				// Detect Adlib channels here (except for OpenMPT 1.19 and older, which would write wrong channel types for PCM channels 16-32):
				// c = channels[i] ^ 0x80;
				// if(c >= 16 && c < 32) adlibChannel = true;
			}
		}
	}
	if(m_nChannels < 1)
	{
		m_nChannels = 1;
	}

	Order.ReadAsByte(file, fileHeader.ordNum);

	// Read sample header offsets
	vector<uint16> sampleOffsets(fileHeader.smpNum);
	for(size_t i = 0; i < fileHeader.smpNum; i++)
	{
		sampleOffsets[i] = file.ReadUint16LE();
	}

	// Read pattern offsets
	vector<uint16> patternOffsets(fileHeader.patNum);
	for(size_t i = 0; i < fileHeader.patNum; i++)
	{
		patternOffsets[i] = file.ReadUint16LE();
	}

	// Read extended channel panning
	if(fileHeader.usePanningTable == S3MFileHeader::idPanning)
	{
		uint8 pan[32];
		file.ReadArray(pan);
		for(CHANNELINDEX i = 0; i < 32; i++)
		{
			if((pan[i] & 0x20) != 0)
			{
				ChnSettings[i].nPan = (static_cast<uint16>(pan[i] & 0x0F) * 256 + 8) / 15;
			}
		}
	}

	bool hasAdlibPatches = false;

	// Reading sample headers
	m_nSamples = min(fileHeader.smpNum, MAX_SAMPLES - 1);
	for(SAMPLEINDEX smp = 0; smp < m_nSamples; smp++)
	{
		S3MSampleHeader sampleHeader;

		if(!file.Seek(sampleOffsets[smp] * 16) || !file.ReadConvertEndianness(sampleHeader))
		{
			continue;
		}

		StringFixer::ReadString<StringFixer::nullTerminated>(m_szNames[smp + 1], sampleHeader.name);
		sampleHeader.ConvertToMPT(Samples[smp + 1]);

		if(sampleHeader.sampleType >= S3MSampleHeader::typeAdMel)
		{
			hasAdlibPatches = true;
		}

		const uint32 sampleOffset = (sampleHeader.dataPointer[1] << 4) | (sampleHeader.dataPointer[2] << 12) | (sampleHeader.dataPointer[0] << 20);

		if(sampleHeader.length != 0 && file.Seek(sampleOffset))
		{
			UINT flags = (fileHeader.formatVersion == S3MFileHeader::oldVersion) ? RS_PCM8S : RS_PCM8U;
			if(sampleHeader.flags & S3MSampleHeader::smp16Bit)
			{
				flags += 5;
			}
			if(sampleHeader.flags & S3MSampleHeader::smpStereo)
			{
				flags |= RSF_STEREO;
			}
			if(sampleHeader.pack == S3MSampleHeader::pADPCM)
			{
				flags = RS_ADPCM4;	// MODPlugin :(
			}

			ReadSample(&Samples[smp + 1], flags, file);
		}
	}

#ifdef MODPLUG_TRACKER
	if(hasAdlibPatches && GetpModDoc() != nullptr)
	{
		GetpModDoc()->AddToLog("This track uses Adlib instruments, which are not supported by OpenMPT.");
	}
#endif // MODPLUG_TRACKER


	// Try to find out if Zxx commands are supposed to be panning commands (PixPlay).
	// Actually I am only aware of one module that uses this panning style, namely "Crawling Despair" by $volkraq
	// and I have no idea what PixPlay is, so this code is solely based on the sample text of that module.
	// We won't convert if there are not enough Zxx commands, too "high" Zxx commands
	// or there are only "left" or "right" pannings (we assume that stereo should be somewhat balanced),
	// and modules not made with an old version of ST3 were probably made in a tracker that supports panning anyway.
	bool pixPlayPanning = (fileHeader.cwtv  < S3MFileHeader::trkST3_20);
	int zxxCountRight = 0, zxxCountLeft = 0;

	// Reading patterns
	const PATTERNINDEX readPatterns = min(fileHeader.patNum, MAX_PATTERNS);
	for(PATTERNINDEX pat = 0; pat < readPatterns; pat++)
	{
		if(!file.Seek(patternOffsets[pat] * 16) || Patterns.Insert(pat, 64))
		{
			continue;
		}

		// Skip pattern length indication.
		// Some modules, for example http://aminet.net/mods/8voic/s3m_hunt.lha seem to have a wrong pattern length -
		// If you strictly adhere the pattern length, you won't read the patterns correctly in that module.
		file.Skip(2);

		// Read pattern data
		ROWINDEX row = 0;
		PatternRow rowBase = Patterns[pat].GetRow(0);

		while(row < 64)
		{
			uint8 info = file.ReadUint8();

			if(info == s3mEndOfRow)
			{
				// End of row
				if(++row < 64)
				{
					rowBase = Patterns[pat].GetRow(row);
				}
				continue;
			}

			CHANNELINDEX channel = (info & s3mChannelMask);
			static ModCommand dummy;
			ModCommand &m = (channel < GetNumChannels()) ? rowBase[channel] : dummy;

			if(info & s3mNotePresent)
			{
				uint8 note = file.ReadUint8(), instr = file.ReadUint8();

				if(note < 0xF0)
				{
					// Note
					note = (note & 0x0F) + 12 * (note >> 4) + 12 + NOTE_MIN;
				} else if(note == s3mNoteOff)
				{
					// ^^
					note = NOTE_NOTECUT;
				} else if(note == s3mNoteNone)
				{
					// ..
					note = NOTE_NONE;
				}

				m.note = note;
				m.instr = instr;
			}

			if(info & s3mVolumePresent)
			{
				uint8 volume = file.ReadUint8();
				if(volume >= 128 && volume <= 192)
				{
					m.volcmd = VOLCMD_PANNING;
					m.vol = volume - 128;
				} else
				{
					m.volcmd = VOLCMD_VOLUME;
					m.vol = min(volume, 64);
				}
			}

			if(info & s3mEffectPresent)
			{
				uint8 command = file.ReadUint8(), param = file.ReadUint8();

				if(command != 0)
				{
					m.command = command;
					m.param = param;
					S3MConvert(m, false);
				}

				if(m.command == CMD_MIDI)
				{
					// PixPlay panning test
					if(m.param > 0x0F)
					{
						// PixPlay has Z00 to Z0F panning, so we ignore this.
						pixPlayPanning = false;
					} else
					{
						if(m.param < 0x08)
						{
							zxxCountLeft++;
						} else  if(m.param > 0x08)
						{
							zxxCountRight++;
						}
					}
				}
			}
		}
	}

	if(pixPlayPanning && zxxCountLeft + zxxCountRight >= m_nChannels && (-zxxCountLeft + zxxCountRight) < static_cast<int>(m_nChannels))
	{
		// There are enough Zxx commands, so let's assume this was made to be played with PixPlay
		Patterns.ForEachModCommand(FixPixPlayPanning());
	}

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveS3M(LPCSTR lpszFileName) const
//-------------------------------------------------
{
	static const uint8 filler[16] =
	{
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	};

	FILE *f;
	if(m_nChannels == 0 || lpszFileName == nullptr) return false;
	if((f = fopen(lpszFileName, "wb")) == nullptr) return false;

	S3MFileHeader fileHeader;
	MemsetZero(fileHeader);

	StringFixer::WriteString<StringFixer::nullTerminated>(fileHeader.name, m_szNames[0]);
	fileHeader.dosEof = S3MFileHeader::idEOF;
	fileHeader.fileType = S3MFileHeader::idS3MType;

	// Orders
	ORDERINDEX writeOrders = Order.GetLengthTailTrimmed();
	if(writeOrders < 2)
	{
		writeOrders = 2;
	} else if((writeOrders % 2) != 0)
	{
		// Number of orders should be even
		writeOrders++;
	}
	LimitMax(writeOrders, static_cast<ORDERINDEX>(256));
	fileHeader.ordNum = static_cast<uint16>(writeOrders);

	// Samples
	SAMPLEINDEX writeSamples = static_cast<SAMPLEINDEX>(GetNumInstruments());
	if(fileHeader.smpNum == 0)
	{
		writeSamples = GetNumSamples();
	}
	writeSamples = Clamp(writeSamples, static_cast<SAMPLEINDEX>(1), static_cast<SAMPLEINDEX>(99));
	fileHeader.smpNum = static_cast<uint16>(writeSamples);

	// Patterns
	PATTERNINDEX writePatterns = min(Patterns.GetNumPatterns(), 100u);
	fileHeader.patNum = static_cast<uint16>(writePatterns);

	// Flags
	if(m_dwSongFlags & SONG_FASTVOLSLIDES)
	{
		fileHeader.flags |= S3MFileHeader::fastVolumeSlides;
	}
	if(m_nMaxPeriod < 20000 || (m_dwSongFlags & SONG_AMIGALIMITS))
	{
		fileHeader.flags |= S3MFileHeader::amigaLimits;
	}

	// Version info following: ST3.20 = 0x1320
	// Most significant nibble = Tracker ID, see S3MFileHeader::S3MTrackerVersions
	// Following: One nibble = Major version, one byte = Minor version (hex)
	fileHeader.cwtv = S3MFileHeader::trkOpenMPT | static_cast<uint16>((MptVersion::num >> 16) & S3MFileHeader::versionMask);
	fileHeader.formatVersion = S3MFileHeader::newVersion;
	fileHeader.magic = S3MFileHeader::idSCRM;

	// Song Variables
	fileHeader.globalVol = static_cast<uint8>(min(m_nDefaultGlobalVolume / 4, 64));
	fileHeader.speed = static_cast<uint8>(Clamp(m_nDefaultSpeed, 1u, 254u));
	fileHeader.tempo = static_cast<uint8>(Clamp(m_nDefaultTempo, 33u, 255u));
	fileHeader.masterVolume = static_cast<uint8>(Clamp(m_nSamplePreAmp, 16u, 127u) | 0x80);
	fileHeader.ultraClicks = 8;
	fileHeader.usePanningTable = S3MFileHeader::idPanning;

	// Channel Table
	for(CHANNELINDEX chn = 0; chn < 32; chn++)
	{
		if(chn < GetNumChannels())
		{
			// ST3 only supports 16 PCM channels, so if channels 17-32 are used,
			// they must be mapped to the same "internal channels" as channels 1-16.
			uint8 ch = ((chn << 3) | (chn >> 1)) & 0x0F;
			if((ChnSettings[chn].dwFlags & CHN_MUTE) != 0)
			{
				ch |= 0x80;
			}
			fileHeader.channels[chn] = ch;
		} else
		{
			fileHeader.channels[chn] = 0xFF;
		}
	}

	fileHeader.ConvertEndianness();
	fwrite(&fileHeader, sizeof(fileHeader), 1, f);
	Order.WriteAsByte(f, writeOrders);

	// Comment about parapointers stolen from Schism Tracker:
	// The sample data parapointers are 24+4 bits, whereas pattern data and sample headers are only 16+4
	// bits -- so while the sample data can be written up to 268 MB within the file (starting at 0xffffff0),
	// the pattern data and sample headers are restricted to the first 1 MB (starting at 0xffff0). In effect,
	// this practically requires the sample data to be written last in the file, as it is entirely possible
	// (and quite easy, even) to write more than 1 MB of sample data in a file.
	// The "practical standard order" listed in TECH.DOC is sample headers, patterns, then sample data.

	// Calculate offset of first sample header...
	size_t sampleHeaderOffset = ftell(f) + (writeSamples + writePatterns) * 2 + 32;
	// ...which must be a multiple of 16, because parapointers omit the lowest 4 bits.
	sampleHeaderOffset = (sampleHeaderOffset + 15) & ~15;

	vector<uint16> sampleOffsets(writeSamples);
	for(SAMPLEINDEX smp = 0; smp < writeSamples; smp++)
	{
		STATIC_ASSERT((sizeof(S3MSampleHeader) % 16) == 0);
		sampleOffsets[smp] = static_cast<uint16>((sampleHeaderOffset + smp * sizeof(S3MSampleHeader)) / 16);
		SwapBytesLE(sampleOffsets[smp]);
	}

	if(writeSamples != 0)
	{
		fwrite(&sampleOffsets[0], 2, writeSamples, f);
	}

	size_t patternPointerOffset = ftell(f);
	size_t firstPatternOffset = sampleHeaderOffset + writeSamples * sizeof(S3MSampleHeader);
	vector<uint16> patternOffsets(writePatterns);
	
	// Need to calculate the real offsets later.
	if(writePatterns != 0)
	{
		fwrite(&patternOffsets[0], 2, writePatterns, f);
	}

	// Write channel panning
	uint8 chnPan[32];
	for(CHANNELINDEX pat = 0; pat < 32; pat++)
	{
		if(pat < GetNumChannels())
		{
			chnPan[pat] = static_cast<uint8>(((ChnSettings[pat].nPan * 15 + 128) / 256)) | 0x20;
		} else
		{
			chnPan[pat] = 0x08;
		}
	}
	fwrite(chnPan, 32, 1, f);

	// Do we need to fill up the file with some padding bytes for 16-Byte alignment?
	size_t curPos = ftell(f);
	if(curPos < sampleHeaderOffset)
	{
		ASSERT(sampleHeaderOffset - curPos < 16);
		fwrite(filler, sampleHeaderOffset - curPos, 1, f);
	}

	// Don't write sample headers for now, we are lacking the sample offset data.
	fseek(f, firstPatternOffset, SEEK_SET);

	// Write patterns
	for(PATTERNINDEX pat = 0; pat < writePatterns; pat++)
	{
		ASSERT((ftell(f) % 16) == 0);
		patternOffsets[pat] = static_cast<uint16>(ftell(f) / 16);
		SwapBytesLE(patternOffsets[pat]);

		vector<uint8> buffer;
		buffer.reserve(5 * 1024);
		// Reserve space for length bytes
		buffer.resize(2, 0);

		if(Patterns.IsValidPat(pat))
		{
			for(ROWINDEX row = 0; row < 64; row++)
			{
				if(row >= Patterns[pat].GetNumRows())
				{
					// Invent empty row
					buffer.push_back(s3mEndOfRow);
					continue;
				}

				const PatternRow rowBase = Patterns[pat].GetRow(row);

				CHANNELINDEX writeChannels = min(32, GetNumChannels());
				for(CHANNELINDEX chn = 0; chn < writeChannels; chn++)
				{
					ModCommand &m = rowBase[chn];

					uint8 info = static_cast<uint8>(chn);
					uint8 note = m.note;
					ModCommand::VOLCMD volcmd = m.volcmd;
					uint8 vol = m.vol;
					uint8 command = m.command;
					uint8 param = m.param;

					if(note != NOTE_NONE || m.instr != 0)
					{
						info |= s3mNotePresent;

						if(note == NOTE_NONE)
						{
							// No Note, or note is too low
							note = s3mNoteNone;
						} else if(note >= NOTE_MIN_SPECIAL)
						{
							// Note Cut
							note = s3mNoteOff;
						} else if(note < 12 + NOTE_MIN)
						{
							// Too low
							note = 0;
						} else if(note <= NOTE_MAX)
						{
							note -= (12 + NOTE_MIN);
							note = (note % 12) + ((note / 12) << 4);
						}
					}

					if(command == CMD_VOLUME)
					{
						command = CMD_NONE;
						volcmd = VOLCMD_VOLUME;
						vol = min(param, 64);
					}

					if(volcmd == VOLCMD_VOLUME)
					{
						info |= s3mVolumePresent;
					} else if(volcmd == VOLCMD_PANNING)
					{
						info |= s3mVolumePresent;
						vol |= 0x80;
					}

					if(command != CMD_NONE)
					{
						S3MSaveConvert(command, param, false, true);
						if(command)
						{
							info |= s3mEffectPresent;
						}
					}

					if(info & s3mAnyPresent)
					{
						buffer.push_back(info);
						if(info & s3mNotePresent)
						{
							buffer.push_back(note);
							buffer.push_back(m.instr);
						}
						if(info & s3mVolumePresent)
						{
							buffer.push_back(vol);
						}
						if(info & s3mEffectPresent)
						{
							buffer.push_back(command);
							buffer.push_back(param);
						}
					}
				}
				
				buffer.push_back(s3mEndOfRow);
			}
		} else
		{
			// Invent empty pattern
			buffer.insert(buffer.end(), 64, s3mEndOfRow);
		}

		size_t length = min(buffer.size(), uint16_max);
		buffer[0] = static_cast<uint8>(length & 0xFF);
		buffer[1] = static_cast<uint8>((length >> 8) & 0xFF);

		if((buffer.size() % 16) != 0)
		{
			// Add padding bytes
			buffer.insert(buffer.end(), 16 - (buffer.size() % 16), 0);
		}

		fwrite(&buffer[0], buffer.size(), 1, f);
	}

	size_t sampleDataOffset = ftell(f);

	// Write samples
	vector<S3MSampleHeader> sampleHeader(writeSamples);

	for(SAMPLEINDEX smp = 0; smp < writeSamples; smp++)
	{
		SAMPLEINDEX realSmp = smp + 1;
		if(GetNumInstruments() != 0 && Instruments[smp] != nullptr)
		{
			// Find some valid sample associated with this instrument.
			for(size_t i = 0; i < CountOf(Instruments[smp]->Keyboard); i++)
			{
				if(Instruments[smp]->Keyboard[i] > 0 && Instruments[smp]->Keyboard[i] <= GetNumSamples())
				{
					realSmp = Instruments[smp]->Keyboard[i];
					break;
				}
			}
		}

		if(realSmp > GetNumSamples())
		{
			continue;
		}

		SmpLength smpLength = sampleHeader[smp].ConvertToS3M(Samples[realSmp]);

		StringFixer::WriteString<StringFixer::nullTerminated>(sampleHeader[smp].name, m_szNames[realSmp]);

		if(Samples[realSmp].pSample)
		{
			// Write sample data
			sampleHeader[smp].dataPointer[1] = static_cast<uint8>((sampleDataOffset >> 4) & 0xFF);
			sampleHeader[smp].dataPointer[2] = static_cast<uint8>((sampleDataOffset >> 12) & 0xFF);
			sampleHeader[smp].dataPointer[0] = static_cast<uint8>((sampleDataOffset >> 20) & 0xFF);

			UINT flags = RS_PCM8U;
			if(Samples[realSmp].uFlags & CHN_16BIT)
			{
				flags = RS_PCM16U;
			}
			if(Samples[realSmp].uFlags & CHN_STEREO)
			{
				flags |= RSF_STEREO;
			}

			UINT writtenLength = WriteSample(f, &Samples[realSmp], flags, smpLength);
			sampleDataOffset += writtenLength;
			if((writtenLength % 16) != 0)
			{
				size_t fillSize = 16 - (writtenLength % 16);
				fwrite(filler, fillSize, 1, f);
				sampleDataOffset += fillSize;
			}
		}

		sampleHeader[smp].ConvertEndianness();
	}

	// Now we know where the patterns are.
	if(writePatterns != 0)
	{
		fseek(f, patternPointerOffset, SEEK_SET);
		fwrite(&patternOffsets[0], 2, writePatterns, f);
	}

	// And we can finally write the sample headers.
	if(writeSamples != 0)
	{
		fseek(f, sampleHeaderOffset, SEEK_SET);
		fwrite(&sampleHeader[0], sizeof(sampleHeader[0]), writeSamples, f);
	}

	fclose(f);
	return true;
}

#endif // MODPLUG_NO_FILESAVE
