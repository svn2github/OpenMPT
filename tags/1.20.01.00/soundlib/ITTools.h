/*
 * ITTools.h
 * ---------
 * Purpose: Definition of IT file structures and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#pragma pack(push, 1)

struct ITFileHeader
{
	// Magic Bytes
	enum Magic
	{
		itMagic = 0x4D504D49,		// "IMPM" IT Header Magic Bytes 
		mptmMagic = 0x2E6D7074,		// "tpm." Old MPTM header magic bytes
		omptMagic = 0x54504D4F,		// "OMPT" Magic Bytes for non-standard OpenMPT IT files
		chibiMagic = 0x49424843,	// "CHBI" Magic Bytes in the IT header to identify ChibiTracker
	};

	// Header Flags
	enum ITHeaderFlags
	{
		useStereoPlayback		= 0x01,
		vol0Optimisations		= 0x02,
		instrumentMode			= 0x04,
		linearSlides			= 0x08,
		itOldEffects			= 0x10,
		itCompatGxx				= 0x20,
		useMIDIPitchController	= 0x40,
		reqEmbeddedMIDIConfig	= 0x80,
		extendedFilterRange		= 0x1000,
	};

	// Special Flags
	enum ITHeaderSpecialFlags
	{
		embedSongMessage		= 0x01,
		embedEditHistory		= 0x02,
		embedPatternHighlights	= 0x04,
		embedMIDIConfiguration	= 0x08,
	};

	uint32 id;				// Magic Bytes (IMPM)
	char   songname[26];	// Song Name, null-terminated (but may also contain nulls)
	uint8  highlight_minor;	// Rows per Beat highlight
	uint8  highlight_major;	// Rows per Measure highlight
	uint16 ordnum;			// Number of Orders
	uint16 insnum;			// Number of Instruments
	uint16 smpnum;			// Number of Samples
	uint16 patnum;			// Number of Patterns
	uint16 cwtv;			// "Made With" Tracker
	uint16 cmwt;			// "Compatible With" Tracker
	uint16 flags;			// Header Flags
	uint16 special;			// Special Flags, for embedding extra information
	uint8  globalvol;		// Global Volume (0...128)
	uint8  mv;				// Master Volume (0...128), referred to as Sample Volume in OpenMPT
	uint8  speed;			// Initial Speed (1...255)
	uint8  tempo;			// Initial Tempo (31...255)
	uint8  sep;				// Pan Separation (0...128)
	uint8  pwd;				// Pitch Wheel Depth
	uint16 msglength;		// Length of Song Message
	uint32 msgoffset;		// Offset of Song Message in File (IT crops message after first null)
	uint32 reserved;		// ChibiTracker writes "CHBI" here. OpenMPT writes "OMPT" here in some cases, see Load_it.cpp
	uint8  chnpan[64];		// Initial Channel Panning
	uint8  chnvol[64];		// Initial Channel Volume

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness();
};

STATIC_ASSERT(sizeof(ITFileHeader) == 192);


struct ITEnvelope
{
	// Envelope Flags
	enum ITEnvelopeFlags
	{
		envEnabled	= 0x01,
		envLoop		= 0x02,
		envSustain	= 0x04,
		envCarry	= 0x08,
		envFilter	= 0x80,
	};

	uint8 flags;			// Envelope Flags
	uint8 num;			// Number of Envelope Nodes
	uint8 lpb;			// Loop Start
	uint8 lpe;			// Loop End
	uint8 slb;			// Sustain Start
	uint8 sle;			// Sustain End
	uint8 data[25 * 3];	// Envelope Node Positions / Values
	uint8 reserved;		// Reserved

	// Convert OpenMPT's internal envelope format to an IT/MPTM envelope.
	void ConvertToIT(const InstrumentEnvelope &mptEnv, BYTE envOffset, BYTE envDefault);
	// Convert IT/MPTM envelope data into OpenMPT's internal envelope format - To be used by ITInstrToMPT()
	void ConvertToMPT(InstrumentEnvelope &mptEnv, BYTE envOffset, int maxNodes) const;
};

STATIC_ASSERT(sizeof(ITEnvelope) == 82);


// Old Impulse Instrument Format (cmwt < 0x200)
struct ITOldInstrument
{
	// Magic Bytes
	enum Magic
	{
		magic = 0x49504D49,	// "IMPI" IT Instrument Header Magic Bytes
	};

	enum ITOldInstrFlags
	{
		envEnabled	= 0x01,
		envLoop		= 0x02,
		envSustain	= 0x04,
	};

	uint32 id;				// Magic Bytes (IMPI)
	char   filename[13];	// DOS Filename, null-terminated
	uint8  flags;			// Volume Envelope Flags
	uint8  vls;				// Envelope Loop Start
	uint8  vle;				// Envelope Loop End
	uint8  sls;				// Envelope Sustain Start
	uint8  sle;				// Envelope Sustain End
	char   reserved1[2];	// Reserved
	uint16 fadeout;			// Instrument Fadeout (0...128)
	uint8  nna;				// New Note Action
	uint8  dnc;				// Duplicate Note Check Type
	uint16 trkvers;			// Tracker ID
	uint8  nos;				// Number of embedded samples
	char   reserved2;		// Reserved
	char   name[26];		// Instrument Name, null-terminated (but may also contain nulls)
	char   reserved3[6];	// Even more reserved bytes
	uint8  keyboard[240];	// Sample / Transpose map
	uint8  volenv[200];		// This appears to be a pre-computed (interpolated) version of the volume envelope data found below.
	uint8  nodes[25 * 2];	// Volume Envelope Node Positions / Values

	// Convert an ITOldInstrument to OpenMPT's internal instrument representation.
	void ConvertToMPT(ModInstrument &mptIns) const;
};

STATIC_ASSERT(sizeof(ITOldInstrument) == 554);


// Impulse Instrument Format
struct ITInstrument
{
	// Magic Bytes
	enum Magic
	{
		magic = 0x49504D49,	// "IMPI" IT Instrument Header Magic Bytes
	};

	enum ITInstrumentFlags
	{
		ignorePanning	= 0x80,
		enableCutoff	= 0x80,
		enableResonance	= 0x80,
	};

	uint32 id;				// Magic Bytes (IMPI)
	char   filename[13];	// DOS Filename, null-terminated
	uint8  nna;				// New Note Action
	uint8  dct;				// Duplicate Note Check Type
	uint8  dca;				// Duplicate Note Check Action
	uint16 fadeout;			// Instrument Fadeout (0...128)
	int8   pps;				// Pitch/Pan Separatation
	uint8  ppc;				// Pitch/Pan Centre
	uint8  gbv;				// Global Volume
	uint8  dfp;				// Panning
	uint8  rv;				// Vol Swing
	uint8  rp;				// Pan Swing
	uint16 trkvers;			// Tracker ID
	uint8  nos;				// Number of embedded samples
	char   reserved1;		// Reserved
	char   name[26];		// Instrument Name, null-terminated (but may also contain nulls)
	uint8  ifc;				// Filter Cutoff
	uint8  ifr;				// Filter Resonance
	uint8  mch;				// MIDI Channel
	uint8  mpr;				// MIDI Program
	uint16 mbank;			// MIDI Bank
	uint8  keyboard[240];	// Sample / Transpose map
	ITEnvelope volenv;		// Volume Envelope
	ITEnvelope panenv;		// Pan Envelope
	ITEnvelope pitchenv;	// Pitch / Filter Envelope
	uint32 dummy;			// IT saves some additional padding bytes to match the size of the old instrument format for simplified loading. We use them for some hacks.

	// Convert OpenMPT's internal instrument representation to an ITInstrument. Returns amount of bytes that need to be written.
	size_t ConvertToIT(const ModInstrument &mptIns, bool compatExport, const CSoundFile &sndFile);
	// Convert an ITInstrument to OpenMPT's internal instrument representation. Returns size of the instrument data that has been read.
	size_t ConvertToMPT(ModInstrument &mptIns, MODTYPE fromType) const;
};

STATIC_ASSERT(sizeof(ITInstrument) == 554);


// MPT IT Instrument Extension
struct ITInstrumentEx
{
	enum Magic
	{
		mptx	= 0x5854504D,	// "MPTX" Extended Instrument Header Magic Bytes
	};

	ITInstrument iti;		// Normal IT Instrument
	uint8 keyboardhi[120];	// High Byte of Sample map

	// Convert OpenMPT's internal instrument representation to an ITInstrumentEx. Returns amount of bytes that need to be written.
	size_t ConvertToIT(const ModInstrument &mptIns, bool compatExport, const CSoundFile &sndFile);
	// Convert an ITInstrumentEx to OpenMPT's internal instrument representation. Returns size of the instrument data that has been read.
	size_t ConvertToMPT(ModInstrument &mptIns, MODTYPE fromType) const;
};

STATIC_ASSERT(sizeof(ITInstrumentEx) == sizeof(ITInstrument) + 120);


// IT Sample Format
struct ITSample
{
	// Magic Bytes
	enum Magic
	{
		magic = 0x53504D49,	// "IMPS" IT Sample Header Magic Bytes
	};

	enum ITSampleFlags
	{
		sampleDataPresent	= 0x01,
		sample16Bit			= 0x02,
		sampleStereo		= 0x04,
		sampleCompressed	= 0x08,
		sampleLoop			= 0x10,
		sampleSustain		= 0x20,
		sampleBidiLoop		= 0x40,
		sampleBidiSustain	= 0x80,

		enablePanning		= 0x80,

		cvtSignedSample		= 0x01,
		cvtIT215Compression	= 0x04,
		cvtADPCMSample		= 0xFF,		// MODPlugin :(
	};

	uint32 id;				// Magic Bytes (IMPS)
	char   filename[13];	// DOS Filename, null-terminated
	uint8  gvl;				// Global Volume
	uint8  flags;			// Sample Flags
	uint8  vol;				// Default Volume
	char   name[26];		// Sample Name, null-terminated (but may also contain nulls)
	uint8  cvt;				// Sample Import Format
	uint8  dfp;				// Sample Panning
	uint32 length;			// Sample Length (in samples)
	uint32 loopbegin;		// Sample Loop Begin (in samples)
	uint32 loopend;			// Sample Loop End (in samples)
	uint32 C5Speed;			// C-5 frequency
	uint32 susloopbegin;	// Sample Sustain Begin (in samples)
	uint32 susloopend;		// Sample Sustain End (in samples)
	uint32 samplepointer;	// Pointer to sample data
	uint8  vis;				// Auto-Vibrato Rate (called Sweep in IT)
	uint8  vid;				// Auto-Vibrato Depth
	uint8  vir;				// Auto-Vibrato Sweep (called Rate in IT)
	uint8  vit;				// Auto-Vibrato Type

	// Convert OpenMPT's internal sample representation to an ITSample.
	void ConvertToIT(const ModSample &mptSmp, MODTYPE fromType);
	// Convert an ITSample to OpenMPT's internal sample representation.
	size_t ConvertToMPT(ModSample &mptSmp) const;
	// Retrieve the internal sample format flags for this instrument.
	UINT GetSampleFormat(uint16 cwtv = 0x214) const;
};

STATIC_ASSERT(sizeof(ITSample) == 80);


#ifdef MODPLUG_TRACKER
#include "../mptrack/Moddoc.h"
#endif // MODPLUG_TRACKER

// IT Header extension: Save history
struct ITHistoryStruct
{
	uint16 fatdate;	// DOS / FAT date when the file was opened / created in the editor. For details, read http://msdn.microsoft.com/en-us/library/ms724247(VS.85).aspx
	uint16 fattime;	// DOS / FAT time when the file was opened / created in the editor.
	uint32 runtime;	// The time how long the file was open in the editor, in 1/18.2th seconds. (= ticks of the DOS timer)

#ifdef MODPLUG_TRACKER

	// Convert an ITHistoryStruct to OpenMPT's internal edit history representation
	void ConvertToMPT(FileHistory &mptHistory) const;
	// Convert OpenMPT's internal edit history representation to an ITHistoryStruct
	void ConvertToIT(const FileHistory &mptHistory);

#endif // MODPLUG_TRACKER

};

STATIC_ASSERT(sizeof(ITHistoryStruct) == 8);

#pragma pack(pop)

// MPT stuff
enum MPTHackMagic
{
	magicPatternNames = 0x4D414E50,		// "PNAM" pattern names
	magicChannelNames = 0x4D414E43,		// "CNAM" channel names
};

enum IT_ReaderBitMasks
{
	// pattern row parsing, the channel data is read to obtain
	// number of channels active in the pattern. These bit masks are
	// to blank out sections of the byte of data being read.

	IT_bitmask_patternChanField_c   = 0x7f,
	IT_bitmask_patternChanMask_c    = 0x3f,
	IT_bitmask_patternChanEnabled_c = 0x80,
	IT_bitmask_patternChanUsed_c    = 0x0f
};
