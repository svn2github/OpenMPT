/*
 * SampleFormats.cpp
 * -----------------
 * Purpose: Code for loading various more or less common sample and instrument formats.
 * Notes  : Needs a lot of rewriting.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#include "mod_specifications.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/Moddoc.h"
#include "../mptrack/TrackerSettings.h"
#include "Dlsbank.h"
#endif //MODPLUG_TRACKER
#include "../common/AudioCriticalSection.h"
#ifndef MODPLUG_NO_FILESAVE
#include "../common/mptFileIO.h"
#endif
#include "../common/misc_util.h"
#include "Tagging.h"
#include "ITTools.h"
#include "XMTools.h"
#include "S3MTools.h"
#include "WAVTools.h"
#include "../common/version.h"
#include "Loaders.h"
#include "ChunkReader.h"
#ifndef NO_OGG
#include <ogg/ogg.h>
#endif // !NO_OGG
#ifndef NO_FLAC
#define FLAC__NO_DLL
#include <flac/include/FLAC/stream_decoder.h>
#include <flac/include/FLAC/stream_encoder.h>
#include <flac/include/FLAC/metadata.h>
#include "SampleFormatConverters.h"
#endif // !NO_FLAC
#include "../common/ComponentManager.h"
#if defined(MPT_WITH_MPG123)
#include "mpg123.h"
#endif // MPT_WITH_MPG123
#if !defined(NO_MEDIAFOUNDATION)
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <Propvarutil.h>
#endif // !NO_MEDIAFOUNDATION
#ifndef NO_MP3_SAMPLES
#include "MPEGFrame.h"
#endif // NO_MP3_SAMPLES

OPENMPT_NAMESPACE_BEGIN


bool CSoundFile::ReadSampleFromFile(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize, bool includeInstrumentFormats)
//--------------------------------------------------------------------------------------------------------------------------
{
	if(!nSample || nSample >= MAX_SAMPLES) return false;
	if(!ReadWAVSample(nSample, file, mayNormalize)
		&& !(includeInstrumentFormats && ReadXISample(nSample, file))
		&& !(includeInstrumentFormats && ReadITISample(nSample, file))
		&& !ReadAIFFSample(nSample, file, mayNormalize)
		&& !ReadITSSample(nSample, file)
		&& !(includeInstrumentFormats && ReadPATSample(nSample, file))
		&& !ReadIFFSample(nSample, file)
		&& !ReadS3ISample(nSample, file)
		&& !ReadFLACSample(nSample, file)
		&& !ReadMP3Sample(nSample, file)
		&& !ReadMediaFoundationSample(nSample, file)
		)
	{
		return false;
	}

	if(nSample > GetNumSamples())
	{
		m_nSamples = nSample;
	}
	return true;
}


bool CSoundFile::ReadInstrumentFromFile(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize)
//--------------------------------------------------------------------------------------------------
{
	if ((!nInstr) || (nInstr >= MAX_INSTRUMENTS)) return false;
	if(!ReadPATInstrument(nInstr, file)
		&& !ReadXIInstrument(nInstr, file)
		&& !ReadITIInstrument(nInstr, file)
		// Generic read
		&& !ReadSampleAsInstrument(nInstr, file, mayNormalize))
	{
		bool ok = false;
#ifdef MODPLUG_TRACKER
		CDLSBank bank;
		if(bank.Open(file))
		{
			ok = bank.ExtractInstrument(*this, nInstr, 0, 0);
		}
#endif // MODPLUG_TRACKER
		if(!ok) return false;
	}

	if(nInstr > GetNumInstruments()) m_nInstruments = nInstr;
	return true;
}


bool CSoundFile::ReadSampleAsInstrument(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize)
//--------------------------------------------------------------------------------------------------
{
	// Scanning free sample
	SAMPLEINDEX nSample = GetNextFreeSample(nInstr); // may also return samples which are only referenced by the current instrument
	if(nSample == SAMPLEINDEX_INVALID)
	{
		return false;
	}

	// Loading Instrument
	ModInstrument *pIns = new (std::nothrow) ModInstrument(nSample);
	if(pIns == nullptr)
	{
		return false;
	}
	if(!ReadSampleFromFile(nSample, file, mayNormalize, false))
	{
		delete pIns;
		return false;
	}

	// Remove all samples which are only referenced by the old instrument, except for the one we just loaded our new sample into.
	RemoveInstrumentSamples(nInstr, nSample);

	// Replace the instrument 
	DestroyInstrument(nInstr, doNoDeleteAssociatedSamples);
	Instruments[nInstr] = pIns;

#if defined(MPT_WITH_FILEIO) && defined(MPT_EXTERNAL_SAMPLES)
	SetSamplePath(nSample, file.GetFileName());
#endif

	return true;
}


bool CSoundFile::DestroyInstrument(INSTRUMENTINDEX nInstr, deleteInstrumentSamples removeSamples)
//-----------------------------------------------------------------------------------------------
{
	if(nInstr == 0 || nInstr >= MAX_INSTRUMENTS || !Instruments[nInstr]) return true;

	if(removeSamples == deleteAssociatedSamples)
	{
		RemoveInstrumentSamples(nInstr);
	}

	CriticalSection cs;

	ModInstrument *pIns = Instruments[nInstr];
	Instruments[nInstr] = nullptr;
	for(CHANNELINDEX i = 0; i < MAX_CHANNELS; i++)
	{
		if (m_PlayState.Chn[i].pModInstrument == pIns)
		{
			m_PlayState.Chn[i].pModInstrument = nullptr;
		}
	}
	delete pIns;
	return true;
}


// Remove all unused samples from the given nInstr and keep keepSample if provided
bool CSoundFile::RemoveInstrumentSamples(INSTRUMENTINDEX nInstr, SAMPLEINDEX keepSample)
//--------------------------------------------------------------------------------------
{
	if(Instruments[nInstr] == nullptr)
	{
		return false;
	}

	std::vector<bool> keepSamples(GetNumSamples() + 1, true);

	// Check which samples are used by the instrument we are going to nuke.
	std::set<SAMPLEINDEX> referencedSamples = Instruments[nInstr]->GetSamples();
	for(std::set<SAMPLEINDEX>::const_iterator sample = referencedSamples.begin(); sample != referencedSamples.end(); sample++)
	{
		if((*sample) <= GetNumSamples())
		{
			keepSamples[*sample] = false;
		}
	}

	// If we want to keep a specific sample, do so.
	if(keepSample != SAMPLEINDEX_INVALID)
	{
		if(keepSample <= GetNumSamples())
		{
			keepSamples[keepSample] = true;
		}
	}

	// Check if any of those samples are referenced by other instruments as well, in which case we want to keep them of course.
	for(INSTRUMENTINDEX nIns = 1; nIns <= GetNumInstruments(); nIns++) if (Instruments[nIns] != nullptr && nIns != nInstr)
	{
		Instruments[nIns]->GetSamples(keepSamples);
	}

	// Now nuke the selected samples.
	RemoveSelectedSamples(keepSamples);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// I/O From another song
//

bool CSoundFile::ReadInstrumentFromSong(INSTRUMENTINDEX targetInstr, const CSoundFile &srcSong, INSTRUMENTINDEX sourceInstr)
//--------------------------------------------------------------------------------------------------------------------------
{
	if ((!sourceInstr) || (sourceInstr > srcSong.GetNumInstruments())
		|| (targetInstr >= MAX_INSTRUMENTS) || (!srcSong.Instruments[sourceInstr]))
	{
		return false;
	}
	if (m_nInstruments < targetInstr) m_nInstruments = targetInstr;

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(targetInstr, deleteAssociatedSamples);

	Instruments[targetInstr] = pIns;
	*pIns = *srcSong.Instruments[sourceInstr];

	std::vector<SAMPLEINDEX> sourceSample;	// Sample index in source song
	std::vector<SAMPLEINDEX> targetSample;	// Sample index in target song
	SAMPLEINDEX targetIndex = 0;		// Next index for inserting sample

	for(size_t i = 0; i < CountOf(pIns->Keyboard); i++)
	{
		const SAMPLEINDEX sourceIndex = pIns->Keyboard[i];
		if(sourceIndex > 0 && sourceIndex <= srcSong.GetNumSamples())
		{
			const std::vector<SAMPLEINDEX>::const_iterator entry = std::find(sourceSample.begin(), sourceSample.end(), sourceIndex);
			if(entry == sourceSample.end())
			{
				// Didn't consider this sample yet, so add it to our map.
				targetIndex = GetNextFreeSample(targetInstr, targetIndex + 1);
				if(targetIndex <= GetModSpecifications().samplesMax)
				{
					sourceSample.push_back(sourceIndex);
					targetSample.push_back(targetIndex);
					pIns->Keyboard[i] = targetIndex;
				} else
				{
					pIns->Keyboard[i] = 0;
				}
			} else
			{
				// Sample reference has already been created, so only need to update the sample map.
				pIns->Keyboard[i] = *(entry - sourceSample.begin() + targetSample.begin());
			}
		} else
		{
			// Invalid or no source sample
			pIns->Keyboard[i] = 0;
		}
	}

	pIns->Convert(srcSong.GetType(), GetType());

	// Copy all referenced samples over
	for(size_t i = 0; i < targetSample.size(); i++)
	{
		ReadSampleFromSong(targetSample[i], srcSong, sourceSample[i]);
	}

	return true;
}


bool CSoundFile::ReadSampleFromSong(SAMPLEINDEX targetSample, const CSoundFile &srcSong, SAMPLEINDEX sourceSample)
//----------------------------------------------------------------------------------------------------------------
{
	if(!sourceSample
		|| sourceSample > srcSong.GetNumSamples()
		|| (targetSample >= GetModSpecifications().samplesMax && targetSample > GetNumSamples()))
	{
		return false;
	}

	DestroySampleThreadsafe(targetSample);

	const ModSample &sourceSmp = srcSong.GetSample(sourceSample);

	if(GetNumSamples() < targetSample) m_nSamples = targetSample;
	Samples[targetSample] = sourceSmp;
	Samples[targetSample].Convert(srcSong.GetType(), GetType());
	strcpy(m_szNames[targetSample], srcSong.m_szNames[sourceSample]);

	if(sourceSmp.pSample)
	{
		Samples[targetSample].pSample = nullptr;	// Don't want to delete the original sample!
		if(Samples[targetSample].AllocateSample())
		{
			SmpLength nSize = sourceSmp.GetSampleSizeInBytes();
			memcpy(Samples[targetSample].pSample, sourceSmp.pSample, nSize);
			Samples[targetSample].PrecomputeLoops(*this, false);
		}
		// Remember on-disk path (for MPTM files), but don't implicitely enable on-disk storage
		// (we really don't want this for e.g. duplicating samples or splitting stereo samples)
#ifdef MPT_EXTERNAL_SAMPLES
		SetSamplePath(targetSample, srcSong.GetSamplePath(sourceSample));
#endif
		Samples[targetSample].uFlags.reset(SMP_KEEPONDISK);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// WAV Open

bool CSoundFile::ReadWAVSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize, FileReader *wsmpChunk)
//-------------------------------------------------------------------------------------------------------------
{
	WAVReader wavFile(file);

	if(!wavFile.IsValid()
		|| wavFile.GetNumChannels() == 0
		|| wavFile.GetNumChannels() > 2
		|| (wavFile.GetBitsPerSample() == 0 && wavFile.GetSampleFormat() != WAVFormatChunk::fmtMP3)
		|| wavFile.GetBitsPerSample() > 32
		|| (wavFile.GetSampleFormat() != WAVFormatChunk::fmtPCM && wavFile.GetSampleFormat() != WAVFormatChunk::fmtFloat && wavFile.GetSampleFormat() != WAVFormatChunk::fmtIMA_ADPCM && wavFile.GetSampleFormat() != WAVFormatChunk::fmtMP3))
	{
		return false;
	}

	DestroySampleThreadsafe(nSample);
	strcpy(m_szNames[nSample], "");
	ModSample &sample = Samples[nSample];
	sample.Initialize();
	sample.nLength = wavFile.GetSampleLength();
	sample.nC5Speed = wavFile.GetSampleRate();
	wavFile.ApplySampleSettings(sample, m_szNames[nSample]);

	FileReader sampleChunk = wavFile.GetSampleData();

	if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtIMA_ADPCM)
	{
		// IMA ADPCM 4:1
		LimitMax(sample.nLength, MAX_SAMPLE_LENGTH);
		sample.uFlags.set(CHN_16BIT);
		sample.uFlags.reset(CHN_STEREO);
		if(!sample.AllocateSample())
		{
			return false;
		}
		IMAADPCMUnpack16(sample.pSample16, sample.nLength, FileReader(sampleChunk.GetRawData(), sampleChunk.BytesLeft()), wavFile.GetBlockAlign());
		sample.PrecomputeLoops(*this, false);
	} else if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtMP3)
	{
		// MP3 in WAV
		return ReadMP3Sample(nSample, sampleChunk);
	} else if(!wavFile.IsExtensibleFormat() && wavFile.MayBeCoolEdit16_8() && wavFile.GetSampleFormat() == WAVFormatChunk::fmtPCM && wavFile.GetBitsPerSample() == 32 && wavFile.GetBlockAlign() == wavFile.GetNumChannels() * 4)
	{
		// Syntrillium Cool Edit hack to store IEEE 32bit floating point
		// Format is described as 32bit integer PCM contained in 32bit blocks and an WAVEFORMATEX extension size of 2 which contains a single 16 bit little endian value of 1.
		//  (This is parsed in WAVTools.cpp and returned via MayBeCoolEdit16_8()).
		// The data actually stored in this case is little endian 32bit floating point PCM with 2**15 full scale.
		// Cool Edit calls this format "16.8 float".
		SampleIO sampleIO(
			SampleIO::_32bit,
			(wavFile.GetNumChannels() > 1) ? SampleIO::stereoInterleaved : SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::floatPCM15);
		sampleIO.ReadSample(sample, sampleChunk);
	} else if(!wavFile.IsExtensibleFormat() && wavFile.GetSampleFormat() == WAVFormatChunk::fmtPCM && wavFile.GetBitsPerSample() == 24 && wavFile.GetBlockAlign() == wavFile.GetNumChannels() * 4)
	{
		// Syntrillium Cool Edit hack to store IEEE 32bit floating point
		// Format is described as 24bit integer PCM contained in 32bit blocks.
		// The data actually stored in this case is little endian 32bit floating point PCM with 2**23 full scale.
		// Cool Edit calls this format "24.0 float".
		SampleIO sampleIO(
			SampleIO::_32bit,
			(wavFile.GetNumChannels() > 1) ? SampleIO::stereoInterleaved : SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::floatPCM23);
		sampleIO.ReadSample(sample, sampleChunk);
	} else
	{
		// PCM / Float
		static const SampleIO::Bitdepth bitDepth[] = { SampleIO::_8bit, SampleIO::_16bit, SampleIO::_24bit, SampleIO::_32bit };

		SampleIO sampleIO(
			bitDepth[(wavFile.GetBitsPerSample() - 1) / 8],
			(wavFile.GetNumChannels() > 1) ? SampleIO::stereoInterleaved : SampleIO::mono,
			SampleIO::littleEndian,
			(wavFile.GetBitsPerSample() > 8) ? SampleIO::signedPCM : SampleIO::unsignedPCM);

		if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtFloat)
		{
			sampleIO |= SampleIO::floatPCM;
		}

		if(mayNormalize)
		{
			sampleIO.MayNormalize();
		}

		sampleIO.ReadSample(sample, sampleChunk);
	}

	if(wsmpChunk != nullptr)
	{
		// DLS WSMP chunk
		*wsmpChunk = wavFile.GetWsmpChunk();
	}

	sample.Convert(MOD_TYPE_IT, GetType());
	sample.PrecomputeLoops(*this, false);

	return true;
}


///////////////////////////////////////////////////////////////
// Save WAV


#ifndef MODPLUG_NO_FILESAVE
bool CSoundFile::SaveWAVSample(SAMPLEINDEX nSample, const mpt::PathString &filename) const
//----------------------------------------------------------------------------------------
{
	mpt::ofstream f(filename, std::ios::binary);
	if(!f)
	{
		return false;
	}

	WAVWriter file(&f);

	if(!file.IsValid())
	{
		return false;
	}

	const ModSample &sample = Samples[nSample];
	file.WriteFormat(sample.GetSampleRate(GetType()), sample.GetElementarySampleSize() * 8, sample.GetNumChannels(), WAVFormatChunk::fmtPCM);

	// Write sample data
	file.StartChunk(RIFFChunk::iddata);
	file.Skip(SampleIO(
		sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
		sample.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		sample.uFlags[CHN_16BIT] ? SampleIO::signedPCM : SampleIO::unsignedPCM)
		.WriteSample(f, sample));

	file.WriteLoopInformation(sample);
	file.WriteExtraInformation(sample, GetType());
	if(sample.HasCustomCuePoints())
	{
		file.WriteCueInformation(sample);
	}
	
	FileTags tags;
	tags.title = mpt::ToUnicode(mpt::CharsetLocale, m_szNames[nSample]);
	tags.encoder = mpt::ToUnicode(mpt::CharsetLocale, MptVersion::GetOpenMPTVersionStr());
	file.WriteMetatags(tags);
	
	return true;
}

#endif // MODPLUG_NO_FILESAVE


#ifndef MODPLUG_NO_FILESAVE

///////////////////////////////////////////////////////////////
// Save RAW

bool CSoundFile::SaveRAWSample(SAMPLEINDEX nSample, const mpt::PathString &filename) const
//----------------------------------------------------------------------------------------
{
	mpt::ofstream f(filename, std::ios::binary);
	if(!f)
	{
		return false;
	}

	const ModSample &sample = Samples[nSample];
	SampleIO(
		sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
		sample.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		SampleIO::signedPCM)
		.WriteSample(f, sample);

	return true;
}

#endif // MODPLUG_NO_FILESAVE

/////////////////////////////////////////////////////////////
// GUS Patches

#ifdef NEEDS_PRAGMA_PACK
#pragma pack(push, 1)
#endif


struct PACKED GF1PatchFileHeader
{
	char   magic[8];		// "GF1PATCH"
	char   version[4];		// "100", or "110"
	char   id[10];			// "ID#000002"
	char   copyright[60];	// Copyright
	uint8  numInstr;		// Number of instruments in patch
	uint8  voices;			// Number of voices, usually 14
	uint8  channels;		// Number of wav channels that can be played concurently to the patch
	uint16 numSamples;		// Total number of waveforms for all the .PAT
	uint16 volume;			// Master volume
	uint32 dataSize;
	char   reserved2[36];

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(numSamples);
		SwapBytesLE(volume);
		SwapBytesLE(dataSize);
	}
};

STATIC_ASSERT(sizeof(GF1PatchFileHeader) == 129);


struct PACKED GF1Instrument
{
	uint16 id;				// Instrument id: 0-65535
	char   name[16];		// Name of instrument. Gravis doesn't seem to use it
	uint32 size;			// Number of bytes for the instrument with header. (To skip to next instrument)
	uint8  layers;			// Number of layers in instrument: 1-4
	char   reserved[40];

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(id);
		SwapBytesLE(size);
	}
};

STATIC_ASSERT(sizeof(GF1Instrument) == 63);


struct PACKED GF1SampleHeader
{
	char name[7];			// null terminated string. name of the wave.
	uint8  fractions;		// Start loop point fraction in 4 bits + End loop point fraction in the 4 other bits.
	uint32 length;			// total size of wavesample. limited to 65535 now by the drivers, not the card.
	uint32 loopstart;		// start loop position in the wavesample
	uint32 loopend;			// end loop position in the wavesample
	uint16 freq;			// Rate at which the wavesample has been sampled
	uint32 low_freq, high_freq, root_freq;	// check note.h for the correspondance.
	int16  finetune;		// fine tune. -512 to +512, EXCLUDING 0 cause it is a multiplier. 512 is one octave off, and 1 is a neutral value
	uint8  balance;			// Balance: 0-15. 0=full left, 15 = full right
	uint8  env_rate[6];		// attack rates
	uint8  env_volume[6];	// attack volumes
	uint8  tremolo_sweep, tremolo_rate, tremolo_depth;
	uint8  vibrato_sweep, vibrato_rate, vibrato_depth;
	uint8  flags;
	int16  scale_frequency;
	uint16 scale_factor;
	char reserved[36];

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(length);
		SwapBytesLE(loopstart);
		SwapBytesLE(loopend);
		SwapBytesLE(freq);
		SwapBytesLE(low_freq);
		SwapBytesLE(high_freq);
		SwapBytesLE(root_freq);
		SwapBytesLE(finetune);
		SwapBytesLE(scale_frequency);
		SwapBytesLE(scale_factor);
	}
};

STATIC_ASSERT(sizeof(GF1SampleHeader) == 96);

// -- GF1 Envelopes --
//
//	It can be represented like this (the envelope is totally bogus, it is
//	just to show the concept):
//
//	|                               
//	|           /----`               | |
//	|   /------/      `\         | | | | |
//	|  /                 \       | | | | |
//	| /                    \     | | | | |
//	|/                       \   | | | | |
//	---------------------------- | | | | | |
//	<---> attack rate 0          0 1 2 3 4 5 amplitudes
//	     <----> attack rate 1
//		     <> attack rate 2
//			 <--> attack rate 3
//			     <> attack rate 4
//				 <-----> attack rate 5
//
// -- GF1 Flags --
//
// bit 0: 8/16 bit
// bit 1: Signed/Unsigned
// bit 2: off/on looping
// bit 3: off/on bidirectionnal looping
// bit 4: off/on backward looping
// bit 5: off/on sustaining (3rd point in env.)
// bit 6: off/on envelopes
// bit 7: off/on clamped release (6th point, env)


struct PACKED GF1Layer
{
	uint8  previous;		// If !=0 the wavesample to use is from the previous layer. The waveheader is still needed
	uint8  id;				// Layer id: 0-3
	uint32 size;			// data size in bytes in the layer, without the header. to skip to next layer for example:
	uint8  samples;			// number of wavesamples
	char   reserved[40];

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesLE(size);
	}
};

STATIC_ASSERT(sizeof(GF1Layer) == 47);


#ifdef NEEDS_PRAGMA_PACK
#pragma pack(pop)
#endif

static int32 PatchFreqToNote(uint32 nFreq)
//----------------------------------------
{
	const float inv_log_2 = 1.44269504089f; // 1.0f/std::log(2.0f)
	const float base = 1.0f / 2044.0f;
	return Util::Round<int32>(std::log(nFreq * base) * (12.0f * inv_log_2));
}


static void PatchToSample(CSoundFile *that, SAMPLEINDEX nSample, GF1SampleHeader &sampleHeader, FileReader &file)
//---------------------------------------------------------------------------------------------------------------
{
	ModSample &sample = that->GetSample(nSample);

	file.ReadConvertEndianness(sampleHeader);

	sample.Initialize();
	if(sampleHeader.flags & 4) sample.uFlags.set(CHN_LOOP);
	if(sampleHeader.flags & 8) sample.uFlags.set(CHN_PINGPONGLOOP);
	sample.nLength = sampleHeader.length;
	sample.nLoopStart = sampleHeader.loopstart;
	sample.nLoopEnd = sampleHeader.loopend;
	sample.nC5Speed = sampleHeader.freq;
	sample.nPan = (sampleHeader.balance * 256 + 8) / 15;
	if(sample.nPan > 256) sample.nPan = 128;
	else sample.uFlags.set(CHN_PANNING);
	sample.nVibType = VIB_SINE;
	sample.nVibSweep = sampleHeader.vibrato_sweep;
	sample.nVibDepth = sampleHeader.vibrato_depth;
	sample.nVibRate = sampleHeader.vibrato_rate / 4;
	sample.FrequencyToTranspose();
	sample.RelativeTone += static_cast<int8>(84 - PatchFreqToNote(sampleHeader.root_freq));
	if(sampleHeader.scale_factor)
	{
		sample.RelativeTone = static_cast<uint8>(sample.RelativeTone - (sampleHeader.scale_frequency - 60));
	}
	sample.TransposeToFrequency();

	SampleIO sampleIO(
		SampleIO::_8bit,
		SampleIO::mono,
		SampleIO::littleEndian,
		(sampleHeader.flags & 2) ? SampleIO::unsignedPCM : SampleIO::signedPCM);

	if(sampleHeader.flags & 1)
	{
		sampleIO |= SampleIO::_16bit;
		sample.nLength /= 2;
		sample.nLoopStart /= 2;
		sample.nLoopEnd /= 2;
	}
	sampleIO.ReadSample(sample, file);
	sample.Convert(MOD_TYPE_IT, that->GetType());
	sample.PrecomputeLoops(*that, false);

	mpt::String::Read<mpt::String::maybeNullTerminated>(that->m_szNames[nSample], sampleHeader.name);
}


bool CSoundFile::ReadPATSample(SAMPLEINDEX nSample, FileReader &file)
//-------------------------------------------------------------------
{
	file.Rewind();
	GF1PatchFileHeader fileHeader;
	GF1Instrument instrHeader;	// We only support one instrument
	GF1Layer layerHeader;
	if(!file.ReadConvertEndianness(fileHeader)
		|| memcmp(fileHeader.magic, "GF1PATCH", 8)
		|| (memcmp(fileHeader.version, "110\0", 4) && memcmp(fileHeader.version, "100\0", 4))
		|| memcmp(fileHeader.id, "ID#000002\0", 10)
		|| !fileHeader.numInstr || !fileHeader.numSamples
		|| !file.ReadConvertEndianness(instrHeader)
		//|| !instrHeader.layers	// DOO.PAT has 0 layers
		|| !file.ReadConvertEndianness(layerHeader)
		|| !layerHeader.samples)
	{
		return false;
	}

	DestroySampleThreadsafe(nSample);
	GF1SampleHeader sampleHeader;
	PatchToSample(this, nSample, sampleHeader, file);

	if(instrHeader.name[0] > ' ')
	{
		mpt::String::Read<mpt::String::maybeNullTerminated>(m_szNames[nSample], instrHeader.name);
	}
	return true;
}


// PAT Instrument
bool CSoundFile::ReadPATInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
//--------------------------------------------------------------------------
{
	file.Rewind();
	GF1PatchFileHeader fileHeader;
	GF1Instrument instrHeader;	// We only support one instrument
	GF1Layer layerHeader;
	if(!file.ReadConvertEndianness(fileHeader)
		|| memcmp(fileHeader.magic, "GF1PATCH", 8)
		|| (memcmp(fileHeader.version, "110\0", 4) && memcmp(fileHeader.version, "100\0", 4))
		|| memcmp(fileHeader.id, "ID#000002\0", 10)
		|| !fileHeader.numInstr || !fileHeader.numSamples
		|| !file.ReadConvertEndianness(instrHeader)
		|| !instrHeader.layers
		|| !file.ReadConvertEndianness(layerHeader)
		|| !layerHeader.samples)
	{
		return false;
	}

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);
	if (nInstr > m_nInstruments) m_nInstruments = nInstr;
	Instruments[nInstr] = pIns;

	mpt::String::Read<mpt::String::maybeNullTerminated>(pIns->name, instrHeader.name);
	pIns->nFadeOut = 2048;
	if(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT))
	{
		pIns->nNNA = NNA_NOTEOFF;
		pIns->nDNA = DNA_NOTEFADE;
	}

	SAMPLEINDEX nextSample = 0;
	int32 nMinSmpNote = 0xFF;
	SAMPLEINDEX nMinSmp = 0;
	for(uint8 smp = 0; smp < layerHeader.samples; smp++)
	{
		// Find a free sample
		nextSample = GetNextFreeSample(nInstr, nextSample + 1);
		if(nextSample == SAMPLEINDEX_INVALID) break;
		if(m_nSamples < nextSample) m_nSamples = nextSample;
		if(!nMinSmp) nMinSmp = nextSample;
		// Load it
		GF1SampleHeader sampleHeader;
		PatchToSample(this, nextSample, sampleHeader, file);
		int32 nMinNote = (sampleHeader.low_freq > 100) ? PatchFreqToNote(sampleHeader.low_freq) : 0;
		int32 nMaxNote = (sampleHeader.high_freq > 100) ? PatchFreqToNote(sampleHeader.high_freq) : NOTE_MAX;
		int32 nBaseNote = (sampleHeader.root_freq > 100) ? PatchFreqToNote(sampleHeader.root_freq) : -1;
		if(!sampleHeader.scale_factor && layerHeader.samples == 1) { nMinNote = 0; nMaxNote = NOTE_MAX; }
		// Fill Note Map
		for(int32 k = 0; k < NOTE_MAX; k++)
		{
			if(k == nBaseNote || (!pIns->Keyboard[k] && k >= nMinNote && k <= nMaxNote))
			{
				if(!sampleHeader.scale_factor)
					pIns->NoteMap[k] = NOTE_MIDDLEC;

				pIns->Keyboard[k] = nextSample;
				if(k < nMinSmpNote)
				{
					nMinSmpNote = k;
					nMinSmp = nextSample;
				}
			}
		}
	}
	if(nMinSmp)
	{
		// Fill note map and missing samples
		for(uint8 k = 0; k < NOTE_MAX; k++)
		{
			if(!pIns->NoteMap[k]) pIns->NoteMap[k] = k + 1;
			if(!pIns->Keyboard[k])
			{
				pIns->Keyboard[k] = nMinSmp;
			} else
			{
				nMinSmp = pIns->Keyboard[k];
			}
		}
	}

	pIns->Convert(MOD_TYPE_IT, GetType());
	return true;
}


/////////////////////////////////////////////////////////////
// S3I Samples


bool CSoundFile::ReadS3ISample(SAMPLEINDEX nSample, FileReader &file)
//-------------------------------------------------------------------
{
	file.Rewind();

	S3MSampleHeader sampleHeader;
	if(!file.ReadConvertEndianness(sampleHeader)
		|| sampleHeader.sampleType != S3MSampleHeader::typePCM
		|| memcmp(sampleHeader.magic, "SCRS", 4)
		|| !file.Seek((sampleHeader.dataPointer[1] << 4) | (sampleHeader.dataPointer[2] << 12) | (sampleHeader.dataPointer[0] << 20)))
	{
		return false;
	}

	DestroySampleThreadsafe(nSample);

	ModSample &sample = Samples[nSample];
	sampleHeader.ConvertToMPT(sample);
	mpt::String::Read<mpt::String::nullTerminated>(m_szNames[nSample], sampleHeader.name);
	sampleHeader.GetSampleFormat(false).ReadSample(sample, file);
	sample.Convert(MOD_TYPE_S3M, GetType());
	sample.PrecomputeLoops(*this, false);
	return true;
}


/////////////////////////////////////////////////////////////
// XI Instruments


bool CSoundFile::ReadXIInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
//-------------------------------------------------------------------------
{
	file.Rewind();

	XIInstrumentHeader fileHeader;
	if(!file.ReadConvertEndianness(fileHeader)
		|| memcmp(fileHeader.signature, "Extended Instrument: ", 21)
		|| fileHeader.version != XIInstrumentHeader::fileVersion
		|| fileHeader.eof != 0x1A)
	{
		return false;
	}

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);
	if(nInstr > m_nInstruments)
	{
		m_nInstruments = nInstr;
	}
	Instruments[nInstr] = pIns;

	fileHeader.ConvertToMPT(*pIns);

	// Translate sample map and find available sample slots
	std::vector<SAMPLEINDEX> sampleMap(fileHeader.numSamples);
	SAMPLEINDEX maxSmp = 0;

	for(size_t i = 0 + 12; i < 96 + 12; i++)
	{
		if(pIns->Keyboard[i] >= fileHeader.numSamples)
		{
			continue;
		}

		if(sampleMap[pIns->Keyboard[i]] == 0)
		{
			// Find slot for this sample
			maxSmp = GetNextFreeSample(nInstr, maxSmp + 1);
			if(maxSmp != SAMPLEINDEX_INVALID)
			{
				sampleMap[pIns->Keyboard[i]] = maxSmp;
			}
		}
		pIns->Keyboard[i] = sampleMap[pIns->Keyboard[i]];
	}

	if(m_nSamples < maxSmp)
	{
		m_nSamples = maxSmp;
	}

	std::vector<SampleIO> sampleFlags(fileHeader.numSamples);

	// Read sample headers
	for(SAMPLEINDEX i = 0; i < fileHeader.numSamples; i++)
	{
		XMSample sampleHeader;
		if(!file.ReadConvertEndianness(sampleHeader)
			|| !sampleMap[i])
		{
			continue;
		}

		ModSample &mptSample = Samples[sampleMap[i]];
		sampleHeader.ConvertToMPT(mptSample);
		fileHeader.instrument.ApplyAutoVibratoToMPT(mptSample);
		mptSample.Convert(MOD_TYPE_XM, GetType());
		if(GetType() != MOD_TYPE_XM && fileHeader.numSamples == 1)
		{
			// No need to pan that single sample, thank you...
			mptSample.uFlags &= ~CHN_PANNING;
		}

		mpt::String::Read<mpt::String::spacePadded>(mptSample.filename, sampleHeader.name);
		mpt::String::Read<mpt::String::spacePadded>(m_szNames[sampleMap[i]], sampleHeader.name);

		sampleFlags[i] = sampleHeader.GetSampleFormat();
	}

	// Read sample data
	for(SAMPLEINDEX i = 0; i < fileHeader.numSamples; i++)
	{
		if(sampleMap[i])
		{
			sampleFlags[i].ReadSample(Samples[sampleMap[i]], file);
			Samples[sampleMap[i]].PrecomputeLoops(*this, false);
		}
	}

	pIns->Convert(MOD_TYPE_XM, GetType());

	// Read MPT crap
	ReadExtendedInstrumentProperties(pIns, file);
	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveXIInstrument(INSTRUMENTINDEX nInstr, const mpt::PathString &filename) const
//----------------------------------------------------------------------------------------------
{
	ModInstrument *pIns = Instruments[nInstr];
	if(pIns == nullptr || filename.empty())
	{
		return false;
	}

	FILE *f;
	if((f = mpt_fopen(filename, "wb")) == nullptr)
	{
		return false;
	}

	// Create file header
	XIInstrumentHeader header;
	header.ConvertToXM(*pIns, false);

	const std::vector<SAMPLEINDEX> samples = header.instrument.GetSampleList(*pIns, false);
	if(samples.size() > 0 && samples[0] <= GetNumSamples())
	{
		// Copy over auto-vibrato settings of first sample
		header.instrument.ApplyAutoVibratoToXM(Samples[samples[0]], GetType());
	}

	header.ConvertEndianness();
	fwrite(&header, 1, sizeof(XIInstrumentHeader), f);

	std::vector<SampleIO> sampleFlags(samples.size());

	// XI Sample Headers
	for(SAMPLEINDEX i = 0; i < samples.size(); i++)
	{
		XMSample xmSample;
		if(samples[i] <= GetNumSamples())
		{
			xmSample.ConvertToXM(Samples[samples[i]], GetType(), false);
		} else
		{
			MemsetZero(xmSample);
		}
		sampleFlags[i] = xmSample.GetSampleFormat();

		mpt::String::Write<mpt::String::spacePadded>(xmSample.name, m_szNames[samples[i]]);

		xmSample.ConvertEndianness();
		fwrite(&xmSample, 1, sizeof(xmSample), f);
	}

	// XI Sample Data
	for(SAMPLEINDEX i = 0; i < samples.size(); i++)
	{
		if(samples[i] <= GetNumSamples())
		{
			sampleFlags[i].WriteSample(f, Samples[samples[i]]);
		}
	}

	int32 code = MAGIC4BE('M','P','T','X');
	fwrite(&code, 1, sizeof(int32), f);		// Write extension tag
	WriteInstrumentHeaderStructOrField(pIns, f);	// Write full extended header.

	fclose(f);
	return true;
}

#endif // MODPLUG_NO_FILESAVE


// Read first sample from XI file into a sample slot
bool CSoundFile::ReadXISample(SAMPLEINDEX nSample, FileReader &file)
//------------------------------------------------------------------
{
	file.Rewind();

	XIInstrumentHeader fileHeader;
	if(!file.ReadConvertEndianness(fileHeader)
		|| !file.CanRead(sizeof(XMSample))
		|| memcmp(fileHeader.signature, "Extended Instrument: ", 21)
		|| fileHeader.version != XIInstrumentHeader::fileVersion
		|| fileHeader.eof != 0x1A
		|| fileHeader.numSamples == 0)
	{
		return false;
	}

	if(m_nSamples < nSample)
	{
		m_nSamples = nSample;
	}

	// Read first sample header
	XMSample sampleHeader;
	file.ReadConvertEndianness(sampleHeader);
	// Gotta skip 'em all!
	file.Skip(sizeof(XMSample) * (fileHeader.numSamples - 1));

	DestroySampleThreadsafe(nSample);

	ModSample &mptSample = Samples[nSample];
	sampleHeader.ConvertToMPT(mptSample);
	if(GetType() != MOD_TYPE_XM)
	{
		// No need to pan that single sample, thank you...
		mptSample.uFlags.reset(CHN_PANNING);
	}
	fileHeader.instrument.ApplyAutoVibratoToMPT(mptSample);
	mptSample.Convert(MOD_TYPE_XM, GetType());

	mpt::String::Read<mpt::String::spacePadded>(mptSample.filename, sampleHeader.name);
	mpt::String::Read<mpt::String::spacePadded>(m_szNames[nSample], sampleHeader.name);

	// Read sample data
	sampleHeader.GetSampleFormat().ReadSample(Samples[nSample], file);
	Samples[nSample].PrecomputeLoops(*this, false);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
// AIFF File I/O


#ifdef NEEDS_PRAGMA_PACK
#pragma pack(push, 1)
#endif


// AIFF header
struct PACKED AIFFHeader
{
	// 32-Bit chunk identifiers
	enum AIFFMagic
	{
		idFORM	= 0x464F524D,
		idAIFF	= 0x41494646,
		idAIFC	= 0x41494643,
	};

	uint32 magic;	// FORM
	uint32 length;	// Size of the file, not including magic and length
	uint32 type;	// AIFF or AIFC

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(magic);
		SwapBytesBE(type);
	}
};

STATIC_ASSERT(sizeof(AIFFHeader) == 12);


// General IFF Chunk header
struct PACKED AIFFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idCOMM	= 0x434F4D4D,
		idSSND	= 0x53534E44,
		idINST	= 0x494E5354,
		idMARK	= 0x4D41524B,
		idNAME	= 0x4E414D45,
	};

	typedef ChunkIdentifiers id_type;

	uint32 id;		// See ChunkIdentifiers
	uint32 length;	// Chunk size without header

	size_t GetLength() const
	{
		return SwapBytesReturnBE(length);
	}

	id_type GetID() const
	{
		return static_cast<id_type>(SwapBytesReturnBE(id));
	}
};

STATIC_ASSERT(sizeof(AIFFChunk) == 8);


// "Common" chunk (in AIFC, a compression ID and compression name follows this header, but apart from that it's identical)
struct PACKED AIFFCommonChunk
{
	uint16 numChannels;
	uint32 numSampleFrames;
	uint16 sampleSize;
	uint8  sampleRate[10];		// Sample rate in 80-Bit floating point

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(numChannels);
		SwapBytesBE(numSampleFrames);
		SwapBytesBE(sampleSize);
	}

	// Convert sample rate to integer
	uint32 GetSampleRate() const
	{
		uint32 mantissa = (sampleRate[2] << 24) | (sampleRate[3] << 16) | (sampleRate[4] << 8) | (sampleRate[5] << 0);
		uint32 last = 0;
		uint8 exp = 30 - sampleRate[1];

		while(exp--)
		{
			last = mantissa;
			mantissa >>= 1;
		}
		if(last & 1) mantissa++;
		return mantissa;
	}
};

STATIC_ASSERT(sizeof(AIFFCommonChunk) == 18);


// Sound chunk
struct PACKED AIFFSoundChunk
{
	uint32 offset;
	uint32 blockSize;

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(offset);
		SwapBytesBE(blockSize);
	}
};

STATIC_ASSERT(sizeof(AIFFSoundChunk) == 8);


// Marker
struct PACKED AIFFMarker
{
	uint16 id;
	uint32 position;		// Position in sample
	uint8  nameLength;		// Not counting eventually existing padding byte in name string

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(id);
		SwapBytesBE(position);
	}};

STATIC_ASSERT(sizeof(AIFFMarker) == 7);


// Instrument loop
struct PACKED AIFFInstrumentLoop
{
	enum PlayModes
	{
		noLoop		= 0,
		loopNormal	= 1,
		loopBidi	= 2,
	};

	uint16 playMode;
	uint16 beginLoop;		// Marker index
	uint16 endLoop;			// Marker index

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(playMode);
		SwapBytesBE(beginLoop);
		SwapBytesBE(endLoop);
	}
};

STATIC_ASSERT(sizeof(AIFFInstrumentLoop) == 6);


struct PACKED AIFFInstrumentChunk
{
	uint8  baseNote;
	uint8  detune;
	uint8  lowNote;
	uint8  highNote;
	uint8  lowVelocity;
	uint8  highVelocity;
	uint16 gain;
	AIFFInstrumentLoop sustainLoop;
	AIFFInstrumentLoop releaseLoop;

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		sustainLoop.ConvertEndianness();
		releaseLoop.ConvertEndianness();
	}
};

STATIC_ASSERT(sizeof(AIFFInstrumentChunk) == 20);


#ifdef NEEDS_PRAGMA_PACK
#pragma pack(pop)
#endif


bool CSoundFile::ReadAIFFSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize)
//---------------------------------------------------------------------------------------
{
	file.Rewind();
	ChunkReader chunkFile(file);

	// Verify header
	AIFFHeader fileHeader;
	if(!chunkFile.ReadConvertEndianness(fileHeader)
		|| fileHeader.magic != AIFFHeader::idFORM
		|| (fileHeader.type != AIFFHeader::idAIFF && fileHeader.type != AIFFHeader::idAIFC))
	{
		return false;
	}

	ChunkReader::ChunkList<AIFFChunk> chunks = chunkFile.ReadChunks<AIFFChunk>(2);

	// Read COMM chunk
	FileReader commChunk(chunks.GetChunk(AIFFChunk::idCOMM));
	AIFFCommonChunk sampleInfo;
	if(!commChunk.ReadConvertEndianness(sampleInfo))
	{
		return false;
	}

	// Is this a proper sample?
	if(sampleInfo.numSampleFrames == 0
		|| sampleInfo.numChannels == 0 || sampleInfo.numChannels > 2
		|| sampleInfo.sampleSize == 0 || sampleInfo.sampleSize > 32)
	{
		return false;
	}

	// Read compression type in AIFF-C files.
	uint8 compression[4] = { 'N', 'O', 'N', 'E' };
	SampleIO::Endianness endian = SampleIO::bigEndian;
	if(fileHeader.type == AIFFHeader::idAIFC)
	{
		if(!commChunk.ReadArray(compression))
		{
			return false;
		}
		if(!memcmp(compression, "twos", 4))
		{
			endian = SampleIO::littleEndian;
		}
	}

	// Read SSND chunk
	FileReader soundChunk(chunks.GetChunk(AIFFChunk::idSSND));
	AIFFSoundChunk sampleHeader;
	if(!soundChunk.ReadConvertEndianness(sampleHeader)
		|| !soundChunk.CanRead(sampleHeader.offset))
	{
		return false;
	}

	static const SampleIO::Bitdepth bitDepth[] = { SampleIO::_8bit, SampleIO::_16bit, SampleIO::_24bit, SampleIO::_32bit };

	SampleIO sampleIO(bitDepth[(sampleInfo.sampleSize - 1) / 8],
		(sampleInfo.numChannels == 2) ? SampleIO::stereoInterleaved : SampleIO::mono,
		endian,
		SampleIO::signedPCM);

	if(!memcmp(compression, "fl32", 4) || !memcmp(compression, "FL32", 4))
	{
		sampleIO |= SampleIO::floatPCM;
	}

	if(mayNormalize)
	{
		sampleIO.MayNormalize();
	}

	soundChunk.Skip(sampleHeader.offset);

	ModSample &mptSample = Samples[nSample];
	DestroySampleThreadsafe(nSample);
	mptSample.Initialize();
	mptSample.nLength = sampleInfo.numSampleFrames;
	mptSample.nC5Speed = sampleInfo.GetSampleRate();

	sampleIO.ReadSample(mptSample, soundChunk);

	// Read MARK and INST chunk to extract sample loops
	FileReader markerChunk(chunks.GetChunk(AIFFChunk::idMARK));
	AIFFInstrumentChunk instrHeader;
	if(markerChunk.IsValid() && chunks.GetChunk(AIFFChunk::idINST).ReadConvertEndianness(instrHeader))
	{
		size_t numMarkers = markerChunk.ReadUint16BE();

		std::vector<AIFFMarker> markers;
		for(size_t i = 0; i < numMarkers; i++)
		{
			AIFFMarker marker;
			if(!markerChunk.ReadConvertEndianness(marker))
			{
				break;
			}
			markers.push_back(marker);
			markerChunk.Skip(marker.nameLength + ((marker.nameLength % 2u) == 0 ? 1 : 0));
		}

		if(instrHeader.sustainLoop.playMode != AIFFInstrumentLoop::noLoop)
		{
			mptSample.uFlags.set(CHN_SUSTAINLOOP);
			mptSample.uFlags.set(CHN_PINGPONGSUSTAIN, instrHeader.sustainLoop.playMode == AIFFInstrumentLoop::loopBidi);
		}

		if(instrHeader.releaseLoop.playMode != AIFFInstrumentLoop::noLoop)
		{
			mptSample.uFlags.set(CHN_LOOP);
			mptSample.uFlags.set(CHN_PINGPONGLOOP, instrHeader.releaseLoop.playMode == AIFFInstrumentLoop::loopBidi);
		}

		// Read markers
		for(std::vector<AIFFMarker>::iterator iter = markers.begin(); iter != markers.end(); iter++)
		{
			if(iter->id == instrHeader.sustainLoop.beginLoop)
			{
				mptSample.nSustainStart = iter->position;
			}
			if(iter->id == instrHeader.sustainLoop.endLoop)
			{
				mptSample.nSustainEnd = iter->position;
			}
			if(iter->id == instrHeader.releaseLoop.beginLoop)
			{
				mptSample.nLoopStart = iter->position;
			}
			if(iter->id == instrHeader.releaseLoop.endLoop)
			{
				mptSample.nLoopEnd = iter->position;
			}
		}
		mptSample.SanitizeLoops();
	}

	// Extract sample name
	FileReader nameChunk(chunks.GetChunk(AIFFChunk::idNAME));
	if(nameChunk.IsValid())
	{
		nameChunk.ReadString<mpt::String::spacePadded>(m_szNames[nSample], nameChunk.GetLength());
	} else
	{
		strcpy(m_szNames[nSample], "");
	}

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
// ITS Samples


bool CSoundFile::ReadITSSample(SAMPLEINDEX nSample, FileReader &file, bool rewind)
//--------------------------------------------------------------------------------
{
	if(rewind)
	{
		file.Rewind();
	}

	ITSample sampleHeader;
	if(!file.ReadConvertEndianness(sampleHeader)
		|| memcmp(sampleHeader.id, "IMPS", 4))
	{
		return false;
	}
	DestroySampleThreadsafe(nSample);

	ModSample &sample = Samples[nSample];
	file.Seek(sampleHeader.ConvertToMPT(sample));
	mpt::String::Read<mpt::String::spacePaddedNull>(m_szNames[nSample], sampleHeader.name);

	if(!sample.uFlags[SMP_KEEPONDISK])
	{
		sampleHeader.GetSampleFormat().ReadSample(Samples[nSample], file);
	} else
	{
		// External sample
		size_t strLen;
		file.ReadVarInt(strLen);
#ifdef MPT_EXTERNAL_SAMPLES
		std::string filenameU8;
		file.ReadString<mpt::String::maybeNullTerminated>(filenameU8, strLen);
		mpt::PathString filename = mpt::PathString::FromUTF8(filenameU8);

		if(!filename.empty())
		{
			if(!file.GetFileName().empty())
			{
				filename = filename.RelativePathToAbsolute(file.GetFileName().GetPath());
			}
			if(!LoadExternalSample(nSample, filename))
			{
				AddToLog(LogWarning, MPT_USTRING("Unable to load sample: ") + filename.ToUnicode());
			}
		} else
		{
			sample.uFlags.reset(SMP_KEEPONDISK);
		}
#else
		file.Skip(strLen);
#endif // MPT_EXTERNAL_SAMPLES
	}

	sample.Convert(MOD_TYPE_IT, GetType());
	sample.PrecomputeLoops(*this, false);
	return true;
}


bool CSoundFile::ReadITISample(SAMPLEINDEX nSample, FileReader &file)
//-------------------------------------------------------------------
{
	ITInstrument instrumentHeader;

	file.Rewind();
	if(!file.ReadConvertEndianness(instrumentHeader)
		|| memcmp(instrumentHeader.id, "IMPI", 4)
		|| instrumentHeader.nos == 0)
	{
		return false;
	}
	file.Rewind();
	ModInstrument dummy;
	ITInstrToMPT(file, dummy, instrumentHeader.trkvers);
	return ReadITSSample(nSample, file, false);
}


bool CSoundFile::ReadITIInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
//--------------------------------------------------------------------------
{
	ITInstrument instrumentHeader;
	SAMPLEINDEX smp = 0, nsamples;

	file.Rewind();
	if(!file.ReadConvertEndianness(instrumentHeader)
		|| memcmp(instrumentHeader.id, "IMPI", 4))
	{
		return false;
	}
	if(nInstr > GetNumInstruments()) m_nInstruments = nInstr;

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);

	Instruments[nInstr] = pIns;
	file.Rewind();
	ITInstrToMPT(file, *pIns, instrumentHeader.trkvers);
	nsamples = instrumentHeader.nos;

	// In order to properly compute the position, in file, of eventual extended settings
	// such as "attack" we need to keep the "real" size of the last sample as those extra
	// setting will follow this sample in the file
	FileReader::off_t extraOffset = file.GetPosition();

	// Reading Samples
	std::vector<SAMPLEINDEX> samplemap(nsamples, 0);
	for(SAMPLEINDEX i = 0; i < nsamples; i++)
	{
		smp = GetNextFreeSample(nInstr, smp + 1);
		if(smp == SAMPLEINDEX_INVALID) break;
		samplemap[i] = smp;
		const FileReader::off_t offset = file.GetPosition();
		ReadITSSample(smp, file, false);
		extraOffset = std::max(extraOffset, file.GetPosition());
		file.Seek(offset + sizeof(ITSample));
	}
	if(GetNumSamples() < smp) m_nSamples = smp;

	for(size_t j = 0; j < CountOf(pIns->Keyboard); j++)
	{
		if(pIns->Keyboard[j] && pIns->Keyboard[j] <= nsamples)
		{
			pIns->Keyboard[j] = samplemap[pIns->Keyboard[j] - 1];
		}
	}

	pIns->Convert(MOD_TYPE_IT, GetType());

	if(file.Seek(extraOffset))
	{
		// Read MPT crap
		ReadExtendedInstrumentProperties(pIns, file);
	}

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveITIInstrument(INSTRUMENTINDEX nInstr, const mpt::PathString &filename, bool compress, bool allowExternal) const
//----------------------------------------------------------------------------------------------------------------------------------
{
	ITInstrumentEx iti;
	ModInstrument *pIns = Instruments[nInstr];
	FILE *f;

	if((!pIns) || filename.empty()) return false;
	if((f = mpt_fopen(filename, "wb")) == NULL) return false;
	
	size_t instSize = iti.ConvertToIT(*pIns, false, *this);

	// Create sample assignment table
	std::vector<SAMPLEINDEX> smptable;
	std::vector<SAMPLEINDEX> smpmap(GetNumSamples(), 0);
	for(size_t i = 0; i < NOTE_MAX; i++)
	{
		const SAMPLEINDEX smp = pIns->Keyboard[i];
		if(smp && smp <= GetNumSamples())
		{
			if(!smpmap[smp - 1])
			{
				// We haven't considered this sample yet.
				smptable.push_back(smp);
				smpmap[smp - 1] = static_cast<SAMPLEINDEX>(smptable.size());
			}
			iti.iti.keyboard[i * 2 + 1] = static_cast<uint8>(smpmap[smp - 1]);
		} else
		{
			iti.iti.keyboard[i * 2 + 1] = 0;
		}
	}
	smpmap.clear();

	uint32 filePos = instSize;
	iti.ConvertEndianness();
	fwrite(&iti, 1, instSize, f);

	filePos += mpt::saturate_cast<uint32>(smptable.size() * sizeof(ITSample));

	// Writing sample headers + data
	std::vector<SampleIO> sampleFlags;
	for(std::vector<SAMPLEINDEX>::iterator iter = smptable.begin(); iter != smptable.end(); iter++)
	{
		ITSample itss;
		itss.ConvertToIT(Samples[*iter], GetType(), compress, compress, allowExternal);
		const bool isExternal = itss.cvt == ITSample::cvtExternalSample;

		mpt::String::Write<mpt::String::nullTerminated>(itss.name, m_szNames[*iter]);

		itss.samplepointer = filePos;
		itss.ConvertEndianness();
		fwrite(&itss, 1, sizeof(itss), f);

		// Write sample
		off_t curPos = ftell(f);
		fseek(f, filePos, SEEK_SET);
		if(!isExternal)
		{
			filePos += mpt::saturate_cast<uint32>(itss.GetSampleFormat(0x0214).WriteSample(f, Samples[*iter]));
		} else
		{
#ifdef MPT_EXTERNAL_SAMPLES
			const std::string filenameU8 = GetSamplePath(*iter).AbsolutePathToRelative(filename.GetPath()).ToUTF8();
			const size_t strSize = mpt::saturate_cast<uint16>(filenameU8.size());
			size_t intBytes = 0;
			if(mpt::IO::WriteVarInt(f, strSize, &intBytes))
			{
				filePos += intBytes + strSize;
				mpt::IO::WriteRaw(f, &filenameU8[0], strSize);
			}
#endif // MPT_EXTERNAL_SAMPLES
		}
		fseek(f, curPos, SEEK_SET);
	}

	fseek(f, 0, SEEK_END);
	int32 code = MAGIC4BE('M','P','T','X');
	SwapBytesLE(code);
	fwrite(&code, 1, sizeof(int32), f);		// Write extension tag
	WriteInstrumentHeaderStructOrField(pIns, f);	// Write full extended header.

	fclose(f);
	return true;
}

#endif // MODPLUG_NO_FILESAVE


///////////////////////////////////////////////////////////////////////////////////////////////////
// 8SVX / 16SVX Samples

#ifdef NEEDS_PRAGMA_PACK
#pragma pack(push, 1)
#endif

// IFF File Header
struct PACKED IFFHeader
{
	char   form[4];		// "FORM"
	uint32 size;
	char   magic[4];	// "8SVX" or "16SV"

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(size);
	}
};

STATIC_ASSERT(sizeof(IFFHeader) == 12);


// General IFF Chunk header
struct PACKED IFFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idVHDR	= 0x52444856,
		idBODY	= 0x59444F42,
		idNAME	= 0x454D414E,
	};

	typedef ChunkIdentifiers id_type;

	uint32 id;		// See ChunkIdentifiers
	uint32 length;	// Chunk size without header

	size_t GetLength() const
	{
		if(length == 0)	// Broken files
			return std::numeric_limits<size_t>::max();
		return SwapBytesReturnBE(length);
	}

	id_type GetID() const
	{
		return static_cast<id_type>(SwapBytesReturnLE(id));
	}
};

STATIC_ASSERT(sizeof(IFFChunk) == 8);


struct PACKED IFFSampleHeader
{
	uint32 oneShotHiSamples;	// Samples in the high octave 1-shot part
	uint32 repeatHiSamples;		// Samples in the high octave repeat part
	uint32 samplesPerHiCycle;	// Samples/cycle in high octave, else 0
	uint16 samplesPerSec;		// Data sampling rate
	uint8  octave;				// Octaves of waveforms
	uint8  compression;			// Data compression technique used
	uint32 volume;

	// Convert all multi-byte numeric values to current platform's endianness or vice versa.
	void ConvertEndianness()
	{
		SwapBytesBE(oneShotHiSamples);
		SwapBytesBE(repeatHiSamples);
		SwapBytesBE(samplesPerHiCycle);
		SwapBytesBE(samplesPerSec);
		SwapBytesBE(volume);
	}
};

STATIC_ASSERT(sizeof(IFFSampleHeader) == 20);

#ifdef NEEDS_PRAGMA_PACK
#pragma pack(pop)
#endif


bool CSoundFile::ReadIFFSample(SAMPLEINDEX nSample, FileReader &file)
//--------------------------------------------------------------------
{
	file.Rewind();

	IFFHeader fileHeader;
	if(!file.ReadConvertEndianness(fileHeader)
		|| memcmp(fileHeader.form, "FORM", 4 )
		|| (memcmp(fileHeader.magic, "8SVX", 4) && memcmp(fileHeader.magic, "16SV", 4)))
	{
		return false;
	}

	ChunkReader chunkFile(file);
	ChunkReader::ChunkList<IFFChunk> chunks = chunkFile.ReadChunks<IFFChunk>(2);

	FileReader vhdrChunk = chunks.GetChunk(IFFChunk::idVHDR);
	FileReader bodyChunk = chunks.GetChunk(IFFChunk::idBODY);
	IFFSampleHeader sampleHeader;
	if(!bodyChunk.IsValid()
		|| !vhdrChunk.IsValid()
		|| !vhdrChunk.ReadConvertEndianness(sampleHeader))
	{
		return false;
	}
	
	DestroySampleThreadsafe(nSample);
	// Default values
	ModSample &sample = Samples[nSample];
	sample.Initialize();
	sample.nLoopStart = sampleHeader.oneShotHiSamples;
	sample.nLoopEnd = sample.nLoopStart + sampleHeader.repeatHiSamples;
	sample.nC5Speed = sampleHeader.samplesPerSec;
	sample.nVolume = static_cast<uint16>(sampleHeader.volume >> 8);
	if(!sample.nVolume || sample.nVolume > 256) sample.nVolume = 256;
	if(!sample.nC5Speed) sample.nC5Speed = 22050;

	sample.Convert(MOD_TYPE_IT, GetType());

	FileReader nameChunk = chunks.GetChunk(IFFChunk::idNAME);
	if(nameChunk.IsValid())
	{
		nameChunk.ReadString<mpt::String::maybeNullTerminated>(m_szNames[nSample], nameChunk.GetLength());
	} else
	{
		strcpy(m_szNames[nSample], "");
	}

	sample.nLength = mpt::saturate_cast<SmpLength>(bodyChunk.GetLength());
	if((sample.nLoopStart + 4 < sample.nLoopEnd) && (sample.nLoopEnd <= sample.nLength)) sample.uFlags.set(CHN_LOOP);

	SampleIO(
		memcmp(fileHeader.magic, "8SVX", 4) ? SampleIO::_16bit : SampleIO::_8bit,
		SampleIO::mono,
		SampleIO::bigEndian,
		SampleIO::signedPCM)
		.ReadSample(sample, bodyChunk);
	sample.PrecomputeLoops(*this, false);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FLAC Samples

#ifndef NO_FLAC

struct FLACDecoder
{
	FileReader &file;
	CSoundFile &sndFile;
	SAMPLEINDEX sample;
	bool ready;

	FLACDecoder(FileReader &f, CSoundFile &sf, SAMPLEINDEX smp) : file(f), sndFile(sf), sample(smp), ready(false) { }

	static FLAC__StreamDecoderReadStatus read_cb(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->file;
		if(*bytes > 0)
		{
			FileReader::off_t readBytes = *bytes;
			LimitMax(readBytes, file.BytesLeft());
			file.ReadRaw(reinterpret_cast<char *>(buffer), readBytes);
			*bytes = readBytes;
			if(*bytes == 0)
				return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
			else
				return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		} else
		{
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		}
	}

	static FLAC__StreamDecoderSeekStatus seek_cb(const FLAC__StreamDecoder *, FLAC__uint64 absolute_byte_offset, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->file;
		if(!file.Seek(static_cast<FileReader::off_t>(absolute_byte_offset)))
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		else
			return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}

	static FLAC__StreamDecoderTellStatus tell_cb(const FLAC__StreamDecoder *, FLAC__uint64 *absolute_byte_offset, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->file;
		*absolute_byte_offset = file.GetPosition();
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}

	static FLAC__StreamDecoderLengthStatus length_cb(const FLAC__StreamDecoder *, FLAC__uint64 *stream_length, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->file;
		*stream_length = file.GetLength();
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}

	static FLAC__bool eof_cb(const FLAC__StreamDecoder *, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->file;
		return file.NoBytesLeft();
	}

	static FLAC__StreamDecoderWriteStatus write_cb(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
	{
		FLACDecoder &client = *static_cast<FLACDecoder *>(client_data);
		ModSample &sample = client.sndFile.GetSample(client.sample);

		if(frame->header.number.sample_number >= sample.nLength || !client.ready)
		{
			// We're reading beyond the sample size already, or we aren't even ready to decode yet!
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}

		// Number of samples to be copied in this call
		const SmpLength copySamples = std::min(static_cast<SmpLength>(frame->header.blocksize), static_cast<SmpLength>(sample.nLength - frame->header.number.sample_number));
		// Number of target channels
		const uint8 modChannels = sample.GetNumChannels();
		// Offset (in samples) into target data
		const size_t offset = static_cast<size_t>(frame->header.number.sample_number) * modChannels;
		// Source size in bytes
		const size_t srcSize = frame->header.blocksize * 4;
		// Source bit depth
		const unsigned int bps = frame->header.bits_per_sample;

		int8 *sampleData8 = sample.pSample8 + offset;
		int16 *sampleData16 = sample.pSample16 + offset;

		MPT_ASSERT((bps <= 8 && sample.GetElementarySampleSize() == 1) || (bps > 8 && sample.GetElementarySampleSize() == 2));
		MPT_ASSERT(modChannels <= FLAC__stream_decoder_get_channels(decoder));
		MPT_ASSERT(bps == FLAC__stream_decoder_get_bits_per_sample(decoder));
		MPT_UNREFERENCED_PARAMETER(decoder); // decoder is unused if ASSERTs are compiled out

		// Do the sample conversion
		for(uint8 chn = 0; chn < modChannels; chn++)
		{
			if(bps <= 8)
			{
				CopySample<SC::ConversionChain<SC::ConvertShift< int8, int32,  0>, SC::DecodeIdentity<int32> > >(sampleData8  + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 16)
			{
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32,  0>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 24)
			{
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32,  8>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 32)
			{
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32, 16>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			}
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	static void metadata_cb(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data)
	{
		FLACDecoder &client = *static_cast<FLACDecoder *>(client_data);
		if(client.sample > client.sndFile.GetNumSamples())
		{
			client.sndFile.m_nSamples = client.sample;
		}
		ModSample &sample = client.sndFile.GetSample(client.sample);

		if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO && metadata->data.stream_info.total_samples != 0)
		{
			// Init sample information
			client.sndFile.DestroySampleThreadsafe(client.sample);
			strcpy(client.sndFile.m_szNames[client.sample], "");
			sample.Initialize();
			sample.uFlags.set(CHN_16BIT, metadata->data.stream_info.bits_per_sample > 8);
			sample.uFlags.set(CHN_STEREO, metadata->data.stream_info.channels > 1);
			sample.nLength = static_cast<SmpLength>(metadata->data.stream_info.total_samples);
			sample.nC5Speed = metadata->data.stream_info.sample_rate;
			client.ready = (sample.AllocateSample() != 0);
		} else if(metadata->type == FLAC__METADATA_TYPE_APPLICATION && !memcmp(metadata->data.application.id, "riff", 4) && client.ready)
		{
			// Try reading RIFF loop points and other sample information
			ChunkReader data(reinterpret_cast<const char*>(metadata->data.application.data), metadata->length);
			ChunkReader::ChunkList<RIFFChunk> chunks = data.ReadChunks<RIFFChunk>(2);

			// We're not really going to read a WAV file here because there will be only one RIFF chunk per metadata event, but we can still re-use the code for parsing RIFF metadata...
			WAVReader riffReader(data);
			riffReader.FindMetadataChunks(chunks);
			riffReader.ApplySampleSettings(sample, client.sndFile.m_szNames[client.sample]);
		} else if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT && client.ready)
		{
			// Try reading Vorbis Comments for sample title, sample rate and loop points
			SmpLength loopStart = 0, loopLength = 0;
			for(FLAC__uint32 i = 0; i < metadata->data.vorbis_comment.num_comments; i++)
			{
				const char *tag = reinterpret_cast<const char *>(metadata->data.vorbis_comment.comments[i].entry);
				const FLAC__uint32 length = metadata->data.vorbis_comment.comments[i].length;
				if(length > 6 && !mpt::CompareNoCaseAscii(tag, "TITLE=", 6))
				{
					mpt::String::Read<mpt::String::maybeNullTerminated>(client.sndFile.m_szNames[client.sample], tag + 6, length - 6);
				} else if(length > 11 && !mpt::CompareNoCaseAscii(tag, "SAMPLERATE=", 11))
				{
					uint32 sampleRate = ConvertStrTo<uint32>(tag + 11);
					if(sampleRate > 0) sample.nC5Speed = sampleRate;
				} else if(length > 10 && !mpt::CompareNoCaseAscii(tag, "LOOPSTART=", 10))
				{
					loopStart = ConvertStrTo<SmpLength>(tag + 10);
				} else if(length > 11 && !mpt::CompareNoCaseAscii(tag, "LOOPLENGTH=", 11))
				{
					loopLength = ConvertStrTo<SmpLength>(tag + 11);
				}
			}
			if(loopLength > 0)
			{
				sample.nLoopStart = loopStart;
				sample.nLoopEnd = loopStart + loopLength;
				sample.uFlags.set(CHN_LOOP);
				sample.SanitizeLoops();
			}
		}
	}

	static void error_cb(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus, void *)
	{
	}
};

#endif // NO_FLAC


bool CSoundFile::ReadFLACSample(SAMPLEINDEX sample, FileReader &file)
//-------------------------------------------------------------------
{
#ifndef NO_FLAC
	file.Rewind();
	bool isOgg = false;
#ifndef NO_OGG
	uint32 oggFlacBitstreamSerial = 0;
#endif
	// Check whether we are dealing with native FLAC, OggFlac or no FLAC at all.
	if(file.ReadMagic("fLaC"))
	{ // ok
		isOgg = false;
#ifndef NO_OGG
	} else if(file.ReadMagic("OggS"))
	{ // use libogg to find the first OggFlac stream header
		file.Rewind();
		bool oggOK = false;
		bool needMoreData = true;
		static const long bufsize = 65536;
		std::size_t readSize = 0;
		char *buf = nullptr;
		ogg_sync_state oy;
		MemsetZero(oy);
		ogg_page og;
		MemsetZero(og);
		std::map<uint32, ogg_stream_state*> oggStreams;
		ogg_packet op;
		MemsetZero(op);
		if(ogg_sync_init(&oy) != 0)
		{
			return false;
		}
		while(needMoreData)
		{
			if(file.NoBytesLeft())
			{ // stop at EOF
				oggOK = false;
				needMoreData = false;
				break;
			}
			buf = ogg_sync_buffer(&oy, bufsize);
			if(!buf)
			{
				oggOK = false;
				needMoreData = false;
				break;
			}
			readSize = file.ReadRaw(buf, bufsize);
			if(ogg_sync_wrote(&oy, static_cast<long>(readSize)) != 0)
			{
				oggOK = false;
				needMoreData = false;
				break;
			}
			while(ogg_sync_pageout(&oy, &og) == 1)
			{
				if(!ogg_page_bos(&og))
				{ // we stop scanning when seeing the first noo-begin-of-stream page
					oggOK = false;
					needMoreData = false;
					break;
				}
				uint32 serial = ogg_page_serialno(&og);
				if(!oggStreams[serial])
				{ // previously unseen stream serial
					oggStreams[serial] = new ogg_stream_state();
					MemsetZero(*(oggStreams[serial]));
					if(ogg_stream_init(oggStreams[serial], serial) != 0)
					{
						delete oggStreams[serial];
						oggStreams.erase(serial);
						oggOK = false;
						needMoreData = false;
						break;
					}
				}
				if(ogg_stream_pagein(oggStreams[serial], &og) != 0)
				{ // invalid page
					oggOK = false;
					needMoreData = false;
					break;
				}
				if(ogg_stream_packetout(oggStreams[serial], &op) != 1)
				{ // partial or broken packet, continue with more data
					continue;
				}
				if(op.packetno != 0)
				{ // non-begin-of-stream packet.
					// This should not appear on first page for any known ogg codec,
					// but deal gracefully with badly mused streams in that regard.
					continue;
				}
				FileReader packet(op.packet, op.bytes);
				if(packet.ReadIntLE<uint8>() == 0x7f && packet.ReadMagic("FLAC"))
				{ // looks like OggFlac
					oggOK = true;
					oggFlacBitstreamSerial = serial;
					needMoreData = false;
					break;
				}
			}
		}
		while(oggStreams.size() > 0)
		{
			uint32 serial = oggStreams.begin()->first;
			ogg_stream_clear(oggStreams[serial]);
			delete oggStreams[serial];
			oggStreams.erase(serial);
		}
		ogg_sync_clear(&oy);
		if(!oggOK)
		{
			return false;
		}
		isOgg = true;
#else // NO_OGG
	} else if(file.CanRead(78) && file.ReadMagic("OggS"))
	{ // first OggFlac page is exactly 78 bytes long
		// only support plain OggFlac here with the FLAC logical bitstream being the first one
		uint8 oggPageVersion = file.ReadIntLE<uint8>();
		uint8 oggPageHeaderType = file.ReadIntLE<uint8>();
		uint64 oggPageGranulePosition = file.ReadIntLE<uint64>();
		uint32 oggPageBitstreamSerialNumber = file.ReadIntLE<uint32>();
		uint32 oggPageSequenceNumber = file.ReadIntLE<uint32>();
		uint32 oggPageChecksum = file.ReadIntLE<uint32>();
		uint8 oggPageSegments = file.ReadIntLE<uint8>();
		uint8 oggPageSegmentLength = file.ReadIntLE<uint8>();
		if(oggPageVersion != 0)
		{ // unknown Ogg version
			return false;
		}
		if(!(oggPageHeaderType & 0x02) || (oggPageHeaderType& 0x01))
		{ // not BOS or continuation
			return false;
		}
		if(oggPageGranulePosition != 0)
		{ // not starting position
			return false;
		}
		if(oggPageSequenceNumber != 0)
		{ // not first page
			return false;
		}
		// skip CRC check for now
		if(oggPageSegments != 1)
		{ // first OggFlac page msut contain exactly 1 segment
			return false;
		}
		if(oggPageSegmentLength != 51)
		{ // segment length must be 51 bytes in OggFlac mapping
			return false;
		}
		if(file.ReadIntLE<uint8>() != 0x7f)
		{ // OggFlac mapping demands 0x7f packet type
			return false;
		}
		if(!file.ReadMagic("FLAC"))
		{ // OggFlac magic
			return false;
		}
		if(file.ReadIntLE<uint8>() != 0x01)
		{ // OggFlac major version
			return false;
		}
		// by now, we are pretty confident that we are not parsing random junk
		isOgg = true;
#endif // !NO_OGG
	} else
	{
		return false;
	}
	file.Rewind();

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if(decoder == nullptr)
	{
		return false;
	}

#ifndef NO_OGG
	if(isOgg)
	{
		// force flac decoding of the logical bitstream that actually is OggFlac
		if(!FLAC__stream_decoder_set_ogg_serial_number(decoder, oggFlacBitstreamSerial))
		{
			FLAC__stream_decoder_delete(decoder);
			return false;
		}
	}
#endif

	// Give me all the metadata!
	FLAC__stream_decoder_set_metadata_respond_all(decoder);

	FLACDecoder client(file, *this, sample);

	// Init decoder
	FLAC__StreamDecoderInitStatus initStatus = isOgg ?
		FLAC__stream_decoder_init_ogg_stream(decoder, FLACDecoder::read_cb, FLACDecoder::seek_cb, FLACDecoder::tell_cb, FLACDecoder::length_cb, FLACDecoder::eof_cb, FLACDecoder::write_cb, FLACDecoder::metadata_cb, FLACDecoder::error_cb, &client)
		:
		FLAC__stream_decoder_init_stream(decoder, FLACDecoder::read_cb, FLACDecoder::seek_cb, FLACDecoder::tell_cb, FLACDecoder::length_cb, FLACDecoder::eof_cb, FLACDecoder::write_cb, FLACDecoder::metadata_cb, FLACDecoder::error_cb, &client)
		;
	if(initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		FLAC__stream_decoder_delete(decoder);
		return false;
	}

	// Decode file
	FLAC__stream_decoder_process_until_end_of_stream(decoder);
	FLAC__stream_decoder_finish(decoder);
	FLAC__stream_decoder_delete(decoder);

	if(client.ready && Samples[sample].pSample != nullptr)
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
		return true;
	}
#else
	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
#endif // NO_FLAC
	return false;
}


#ifndef NO_FLAC
// Helper function for copying OpenMPT's sample data to FLAC's int32 buffer.
template<typename T>
inline static void SampleToFLAC32(FLAC__int32 *dst, const T *src, SmpLength numSamples)
{
	for(SmpLength i = 0; i < numSamples; i++)
	{
		dst[i] = src[i];
	}
};

// RAII-style helper struct for FLAC encoder
struct FLAC__StreamEncoder_RAII
{
	FLAC__StreamEncoder *encoder;
	FILE *f;

	operator FLAC__StreamEncoder *() { return encoder; }

	FLAC__StreamEncoder_RAII() : encoder(FLAC__stream_encoder_new()), f(nullptr) { }
	~FLAC__StreamEncoder_RAII()
	{
		FLAC__stream_encoder_delete(encoder);
		if(f != nullptr) fclose(f);
	}
};

#endif


#ifndef MODPLUG_NO_FILESAVE
bool CSoundFile::SaveFLACSample(SAMPLEINDEX nSample, const mpt::PathString &filename) const
//-----------------------------------------------------------------------------------------
{
#ifndef NO_FLAC
	FLAC__StreamEncoder_RAII encoder;
	if(encoder == nullptr)
	{
		return false;
	}

	const ModSample &sample = Samples[nSample];
	uint32 sampleRate = sample.GetSampleRate(GetType());

	// First off, set up all the metadata...
	FLAC__StreamMetadata *metadata[] =
	{
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT),
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// MPT sample information
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// Loop points
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// Cue points
	};

	unsigned numBlocks = 2;
	if(metadata[0])
	{
		// Store sample name
		FLAC__StreamMetadata_VorbisComment_Entry entry;
		FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "TITLE", m_szNames[nSample]);
		FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ENCODER", MptVersion::GetOpenMPTVersionStr().c_str());
		FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		if(sampleRate > FLAC__MAX_SAMPLE_RATE)
		{
			// FLAC only supports a sample rate of up to 655350 Hz.
			// Store the real sample rate in a custom Vorbis comment.
			FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "SAMPLERATE", mpt::ToString(sampleRate).c_str());
			FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		}
	}
	if(metadata[1])
	{
		// Write MPT sample information
		memcpy(metadata[1]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			WAVExtraChunk mptInfo;
		} chunk;

		chunk.header.id = RIFFChunk::idxtra;
		chunk.header.length = sizeof(WAVExtraChunk);

		chunk.mptInfo.ConvertToWAV(sample, GetType());

		const uint32 length = sizeof(RIFFChunk) + sizeof(WAVExtraChunk);
		chunk.header.ConvertEndianness();
		chunk.mptInfo.ConvertEndianness();

		FLAC__metadata_object_application_set_data(metadata[1], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
	}
	if(metadata[numBlocks] && sample.uFlags[CHN_LOOP | CHN_SUSTAINLOOP])
	{
		// Store loop points
		memcpy(metadata[numBlocks]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			WAVSampleInfoChunk info;
			WAVSampleLoop loops[2];
		} chunk;

		chunk.header.id = RIFFChunk::idsmpl;
		chunk.header.length = sizeof(WAVSampleInfoChunk);

		chunk.info.ConvertToWAV(sample.GetSampleRate(GetType()));

		if(sample.uFlags[CHN_SUSTAINLOOP])
		{
			chunk.loops[chunk.info.numLoops++].ConvertToWAV(sample.nSustainStart, sample.nSustainEnd, sample.uFlags[CHN_PINGPONGSUSTAIN]);
			chunk.header.length += sizeof(WAVSampleLoop);
		}
		if(sample.uFlags[CHN_LOOP])
		{
			chunk.loops[chunk.info.numLoops++].ConvertToWAV(sample.nLoopStart, sample.nLoopEnd, sample.uFlags[CHN_PINGPONGLOOP]);
			chunk.header.length += sizeof(WAVSampleLoop);
		}

		const uint32 length = sizeof(RIFFChunk) + chunk.header.length;
		chunk.header.ConvertEndianness();
		chunk.info.ConvertEndianness();
		chunk.loops[0].ConvertEndianness();
		chunk.loops[1].ConvertEndianness();
		
		FLAC__metadata_object_application_set_data(metadata[numBlocks], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
		numBlocks++;
	}
	if(metadata[numBlocks] && sample.HasCustomCuePoints())
	{
		// Store cue points
		memcpy(metadata[numBlocks]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			uint32 numPoints;
			WAVCuePoint cues[CountOf(sample.cues)];
		} chunk;

		chunk.header.id = RIFFChunk::idcue_;
		chunk.header.length = 4 + sizeof(chunk.cues);

		for(uint32 i = 0; i < CountOf(sample.cues); i++)
		{
			chunk.cues[i].ConvertToWAV(i, sample.cues[i]);
			chunk.cues[i].ConvertEndianness();
		}

		const uint32 length = sizeof(RIFFChunk) + chunk.header.length;
		chunk.header.ConvertEndianness();

		FLAC__metadata_object_application_set_data(metadata[numBlocks], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
		numBlocks++;
	}

	// FLAC allows a maximum sample rate of 655350 Hz.
	// If the real rate is higher, we store it in a Vorbis comment above.
	LimitMax(sampleRate, FLAC__MAX_SAMPLE_RATE);
	if(!FLAC__format_sample_rate_is_subset(sampleRate))
	{
		// FLAC only supports 10 Hz granularity for frequencies above 65535 Hz if the streamable subset is chosen.
		FLAC__stream_encoder_set_streamable_subset(encoder, false);
	}
	FLAC__stream_encoder_set_channels(encoder, sample.GetNumChannels());
	FLAC__stream_encoder_set_bits_per_sample(encoder, sample.GetElementarySampleSize() * 8);
	FLAC__stream_encoder_set_sample_rate(encoder, sampleRate);
	FLAC__stream_encoder_set_total_samples_estimate(encoder, sample.nLength);
	FLAC__stream_encoder_set_metadata(encoder, metadata, numBlocks);
#ifdef MODPLUG_TRACKER
	FLAC__stream_encoder_set_compression_level(encoder, TrackerSettings::Instance().m_FLACCompressionLevel);
#endif // MODPLUG_TRACKER

	bool result = false;
	FLAC__int32 *sampleData = nullptr;

	encoder.f = mpt_fopen(filename, "wb");
	if(encoder.f == nullptr || FLAC__stream_encoder_init_FILE(encoder, encoder.f, nullptr, nullptr) != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
	{
		goto fail;
	}

	// Convert sample data to signed 32-Bit integer array.
	const SmpLength numSamples = sample.nLength * sample.GetNumChannels();
	sampleData = new (std::nothrow) FLAC__int32[numSamples];
	if(sampleData == nullptr)
	{
		goto fail;
	}

	if(sample.GetElementarySampleSize() == 1)
	{
		SampleToFLAC32(sampleData, sample.pSample8, numSamples);
	} else if(sample.GetElementarySampleSize() == 2)
	{
		SampleToFLAC32(sampleData, sample.pSample16, numSamples);
	} else
	{
		MPT_ASSERT_NOTREACHED();
	}

	// Do the actual conversion.
	FLAC__stream_encoder_process_interleaved(encoder, sampleData, sample.nLength);
	result = true;
	
fail:
	FLAC__stream_encoder_finish(encoder);

	delete[] sampleData;
	for(size_t i = 0; i < CountOf(metadata); i++)
	{
		FLAC__metadata_object_delete(metadata[i]);
	}

	return result;
#else
	MPT_UNREFERENCED_PARAMETER(nSample);
	MPT_UNREFERENCED_PARAMETER(filename);
	return false;
#endif // NO_FLAC
}
#endif // MODPLUG_NO_FILESAVE


///////////////////////////////////////////////////////////////////////////////////////////////////
// MP3 Samples

#ifndef NO_MP3_SAMPLES

#if !defined(MPT_WITH_MPG123)

	enum mpg123_enc_enum
	{
		MPG123_ENC_16 = 0x040, MPG123_ENC_SIGNED = 0x080,
	};

	typedef struct {int foo;} mpg123_handle;

#endif // MPT_WITH_MPG123

class ComponentMPG123
#if !defined(MPT_WITH_MPG123)
	: public ComponentLibrary
#endif // !MPT_WITH_MPG123
{
#if !defined(MPT_WITH_MPG123)
	MPT_DECLARE_COMPONENT_MEMBERS
#endif // !MPT_WITH_MPG123
public:

	int (*mpg123_init )(void);
	mpg123_handle* (*mpg123_new )(const char*,int*);
	void (*mpg123_delete )(mpg123_handle*);
	int (*mpg123_open_handle )(mpg123_handle*, void*);
#if MPT_COMPILER_MSVC
	int (*mpg123_replace_reader_handle)(mpg123_handle*, 
		size_t(*)(void *, void *, size_t),
		off_t(*)(void *, off_t, int),
		void(*)(void *));
#else // !MPT_COMPILER_MSVC
	int (*mpg123_replace_reader_handle)(mpg123_handle*, 
		ssize_t(*)(void *, void *, size_t),
		off_t(*)(void *, off_t, int),
		void(*)(void *));
#endif // MPT_COMPILER_MSVC
	int (*mpg123_read )(mpg123_handle*, unsigned char*, size_t, size_t*);
	int (*mpg123_getformat )(mpg123_handle*, long*, int*, int*);
	int (*mpg123_scan )(mpg123_handle*);
	off_t (*mpg123_length )(mpg123_handle*);

#if MPT_COMPILER_MSVC
	static size_t FileReaderRead(void *fp, void *buf, size_t count)
#else // !MPT_COMPILER_MSVC
	static ssize_t FileReaderRead(void *fp, void *buf, size_t count)
#endif // MPT_COMPILER_MSVC
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		size_t readBytes = std::min(count, static_cast<size_t>(file.BytesLeft()));
		file.ReadRaw(static_cast<char *>(buf), readBytes);
		return readBytes;
	}
	static off_t FileReaderLSeek(void *fp, off_t offset, int whence)
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		if(whence == SEEK_CUR) file.Seek(file.GetPosition() + offset);
		else if(whence == SEEK_END) file.Seek(file.GetLength() + offset);
		else file.Seek(offset);
		return file.GetPosition();
	}

public:
#if defined(MPT_WITH_MPG123)
	ComponentMPG123()
	{
		#define MPT_GLOBAL_BIND(lib, name) name = &::name;
			MPT_GLOBAL_BIND("mpg123", mpg123_init);
			MPT_GLOBAL_BIND("mpg123", mpg123_new);
			MPT_GLOBAL_BIND("mpg123", mpg123_delete);
			MPT_GLOBAL_BIND("mpg123", mpg123_open_handle);
			MPT_GLOBAL_BIND("mpg123", mpg123_replace_reader_handle);
			MPT_GLOBAL_BIND("mpg123", mpg123_read);
			MPT_GLOBAL_BIND("mpg123", mpg123_getformat);
			MPT_GLOBAL_BIND("mpg123", mpg123_scan);
			MPT_GLOBAL_BIND("mpg123", mpg123_length);
		#undef MPT_GLOBAL_BIND
	}
#else // !MPT_WITH_MPG123
	ComponentMPG123() : ComponentLibrary(ComponentTypeForeign) { }
	bool DoInitialize()
	{
		AddLibrary("mpg123", mpt::LibraryPath::AppFullName(MPT_PATHSTRING("libmpg123-0")));
		AddLibrary("mpg123", mpt::LibraryPath::AppFullName(MPT_PATHSTRING("libmpg123")));
		AddLibrary("mpg123", mpt::LibraryPath::AppFullName(MPT_PATHSTRING("mpg123-0")));
		AddLibrary("mpg123", mpt::LibraryPath::AppFullName(MPT_PATHSTRING("mpg123")));
		MPT_COMPONENT_BIND("mpg123", mpg123_init);
		MPT_COMPONENT_BIND("mpg123", mpg123_new);
		MPT_COMPONENT_BIND("mpg123", mpg123_delete);
		MPT_COMPONENT_BIND("mpg123", mpg123_open_handle);
		MPT_COMPONENT_BIND("mpg123", mpg123_replace_reader_handle);
		MPT_COMPONENT_BIND("mpg123", mpg123_read);
		MPT_COMPONENT_BIND("mpg123", mpg123_getformat);
		MPT_COMPONENT_BIND("mpg123", mpg123_scan);
		MPT_COMPONENT_BIND("mpg123", mpg123_length);
		if(HasBindFailed())
		{
			return false;
		}
		if(mpg123_init() != 0)
		{
			return false;
		}
		return true;
	}
#endif // MPT_WITH_MPG123
};
#if !defined(MPT_WITH_MPG123)
MPT_REGISTERED_COMPONENT(ComponentMPG123, "Mpg123")
#endif // !MPT_WITH_MPG123

#endif // NO_MP3_SAMPLES


bool CSoundFile::ReadMP3Sample(SAMPLEINDEX sample, FileReader &file, bool mo3Decode)
//----------------------------------------------------------------------------------
{
#ifndef NO_MP3_SAMPLES

	// Check file for validity, or else mpg123 will happily munch many files that start looking vaguely resemble an MPEG stream mid-file.
	file.Rewind();
	while(file.CanRead(4))
	{
		uint8 header[3];
		file.ReadArray(header);

		if(!memcmp(header, "ID3", 3))
		{
			// Skip ID3 tags
			uint8 header[7];
			file.ReadArray(header);

			uint32 size = 0;
			for(int i = 3; i < 7; i++)
			{
				if(header[i] & 0x80)
					return false;
				size = (size << 7) | header[i];
			}
			file.Skip(size);
		} else if(!memcmp(header, "APE", 3) && file.ReadMagic("TAGEX"))
		{
			// Skip APE tags
			uint32 size = file.ReadUint32LE();
			file.Skip(16 + size);
		} else if(!memcmp(header, "\x00\x00\x00", 3) || !memcmp(header, "\xFF\x00\x00", 3))
		{
			// Some MP3 files are padded with zeroes...
		} else if(header[0] == 0)
		{
			// This might be some padding, followed by an MPEG header, so try again.
			file.SkipBack(2);
		} else if(MPEGFrame::IsMPEGHeader(header))
		{
			// This is what we want!
			break;
		} else
		{
			// This, on the other hand, isn't.
			return false;
		}
	}

#if defined(MPT_WITH_MPG123)
	ComponentMPG123 mpg123_;
	ComponentMPG123 *mpg123 = &mpg123_;
#else // !MPT_WITH_MPG123
	ComponentHandle<ComponentMPG123> mpg123;
	if(!IsComponentAvailable(mpg123))
	{
		return false;
	}
#endif // MPT_WITH_MPG123

	mpg123_handle *mh;
	int err;
	if((mh = mpg123->mpg123_new(0, &err)) == nullptr) return false;
	file.Rewind();


	long rate; int nchannels, encoding;
	SmpLength length;

	// Set up decoder...
	if(mpg123->mpg123_replace_reader_handle(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderLSeek, 0)
		|| mpg123->mpg123_open_handle(mh, &file)
		|| mpg123->mpg123_scan(mh)
		|| mpg123->mpg123_getformat(mh, &rate, &nchannels, &encoding)
		|| !nchannels || nchannels > 2
		|| (encoding & (MPG123_ENC_16 | MPG123_ENC_SIGNED)) != (MPG123_ENC_16 | MPG123_ENC_SIGNED)
		|| (length = mpg123->mpg123_length(mh)) == 0)
	{
		mpg123->mpg123_delete(mh);
		return false;
	}

	DestroySampleThreadsafe(sample);
	if(!mo3Decode)
	{
		strcpy(m_szNames[sample], "");
		Samples[sample].Initialize();
		Samples[sample].nC5Speed = rate;
	}
	Samples[sample].nLength = length;

	Samples[sample].uFlags.set(CHN_16BIT);
	Samples[sample].uFlags.set(CHN_STEREO, nchannels == 2);
	Samples[sample].AllocateSample();

	size_t ndecoded;
	mpg123->mpg123_read(mh, static_cast<unsigned char *>(Samples[sample].pSample), Samples[sample].GetSampleSizeInBytes(), &ndecoded);
	mpg123->mpg123_delete(mh);

	if(!mo3Decode)
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
	}
	return Samples[sample].pSample != nullptr;
#else
	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(mo3Decode);
#endif // NO_MP3_SAMPLES
	return false;
}


#if !defined(NO_MEDIAFOUNDATION)

template <typename T>
static void mptMFSafeRelease(T **ppT)
{
	if(*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

#define MPT_MF_CHECKED(x) do { \
	HRESULT hr = (x); \
	if(!SUCCEEDED(hr)) \
	{ \
		goto fail; \
	} \
} while(0)

// Implementing IMFByteStream is apparently not enough to stream raw bytes
// data to MediaFoundation.
// Additionally, one has to also implement a custom IMFAsyncResult for the
// BeginRead/EndRead interface which allows transferring the number of read
// bytes around.
// To make things even worse, MediaFoundation fails to detect some AAC and MPEG
// files if a non-file-based or read-only stream is used for opening.
// The only sane option which remains if we do not have an on-disk filename
// available:
//  1 - write a temporary file
//  2 - close it
//  3 - open it using MediaFoundation.
// We use FILE_ATTRIBUTE_TEMPORARY which will try to keep the file data in
// memory just like regular allocated memory and reduce the overhead basically
// to memcpy.

static FileTags ReadMFMetadata(IMFMediaSource *mediaSource)
//---------------------------------------------------------
{

	FileTags tags;

	IMFPresentationDescriptor *presentationDescriptor = NULL;
	DWORD streams = 0;
	IMFMetadataProvider *metadataProvider = NULL;
	IMFMetadata *metadata = NULL;
	PROPVARIANT varPropNames;
	PropVariantInit(&varPropNames);

	MPT_MF_CHECKED(mediaSource->CreatePresentationDescriptor(&presentationDescriptor));
	MPT_MF_CHECKED(presentationDescriptor->GetStreamDescriptorCount(&streams));
	MPT_MF_CHECKED(MFGetService(mediaSource, MF_METADATA_PROVIDER_SERVICE, IID_IMFMetadataProvider, (void**)&metadataProvider));
	MPT_MF_CHECKED(metadataProvider->GetMFMetadata(presentationDescriptor, 0, 0, &metadata));

	MPT_MF_CHECKED(metadata->GetAllPropertyNames(&varPropNames));
	for(DWORD propIndex = 0; propIndex < varPropNames.calpwstr.cElems; ++propIndex)
	{

		PROPVARIANT propVal;
		PropVariantInit(&propVal);

		LPWSTR propName = varPropNames.calpwstr.pElems[propIndex];
		if(S_OK != metadata->GetProperty(propName, &propVal))
		{
			PropVariantClear(&propVal);
			break;
		}

		std::wstring stringVal;
		std::vector<WCHAR> wcharVal(256);
		while(true)
		{
			HRESULT hrToString = PropVariantToString(propVal, &wcharVal[0], wcharVal.size());
			if(hrToString == S_OK)
			{
				stringVal = &wcharVal[0];
				break;
			} else if(hrToString == ERROR_INSUFFICIENT_BUFFER)
			{
				wcharVal.resize(wcharVal.size() * 2);
			} else
			{
				break;
			}
		}

		PropVariantClear(&propVal);

		if(stringVal.length() > 0)
		{
			if(propName == std::wstring(L"Author")) tags.artist = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"Title")) tags.title = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/AlbumTitle")) tags.album = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Track")) tags.trackno = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Year")) tags.year = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Genre")) tags.genre = mpt::ToUnicode(stringVal);
		}
	}

fail:

	PropVariantClear(&varPropNames);
	mptMFSafeRelease(&metadata);
	mptMFSafeRelease(&metadataProvider);
	mptMFSafeRelease(&presentationDescriptor);

	return tags;

}


class ComponentMediaFoundation : public ComponentLibrary
{
	MPT_DECLARE_COMPONENT_MEMBERS
public:
	ComponentMediaFoundation()
		: ComponentLibrary(ComponentTypeSystem)
	{
		return;
	}
	virtual bool DoInitialize()
	{
		if(!mpt::Windows::Version::IsAtLeast(mpt::Windows::Version::Win7))
		{
			return false;
		}
		if(!(true
			&& AddLibrary("mf", mpt::LibraryPath::System(MPT_PATHSTRING("mf")))
			&& AddLibrary("mfplat", mpt::LibraryPath::System(MPT_PATHSTRING("mfplat")))
			&& AddLibrary("mfreadwrite", mpt::LibraryPath::System(MPT_PATHSTRING("mfreadwrite")))
			&& AddLibrary("propsys", mpt::LibraryPath::System(MPT_PATHSTRING("propsys")))
			))
		{
			return false;
		}
		if(!SUCCEEDED(MFStartup(MF_VERSION)))
		{
			return false;
		}
		return true;
	}
	virtual ~ComponentMediaFoundation()
	{
		if(IsAvailable())
		{
			MFShutdown();
		}
	}
};
MPT_REGISTERED_COMPONENT(ComponentMediaFoundation, "MediaFoundation")

#endif // !NO_MEDIAFOUNDATION


#ifdef MODPLUG_TRACKER
#if defined(MPT_WITH_PATHSTRING)
std::vector<FileType> CSoundFile::GetMediaFoundationFileTypes()
//-------------------------------------------------------------
{
	std::vector<FileType> result;

#if !defined(NO_MEDIAFOUNDATION)

	ComponentHandle<ComponentMediaFoundation> mf;
	if(!IsComponentAvailable(mf))
	{
		return result;
	}

	std::map<std::wstring, FileType> guidMap;

	HKEY hkHandlers = NULL;
	LSTATUS regResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers", 0, KEY_READ, &hkHandlers);
	if(regResult != ERROR_SUCCESS)
	{
		return result;
	}

	for(DWORD handlerIndex = 0; ; ++handlerIndex)
	{

		WCHAR handlerTypeBuf[256];
		MemsetZero(handlerTypeBuf);
		regResult = RegEnumKeyW(hkHandlers, handlerIndex, handlerTypeBuf, 256);
		if(regResult != ERROR_SUCCESS)
		{
			break;
		}

		std::wstring handlerType = handlerTypeBuf;

		if(handlerType.length() < 1)
		{
			continue;
		}

		HKEY hkHandler = NULL;
		regResult = RegOpenKeyExW(hkHandlers, handlerTypeBuf, 0, KEY_READ, &hkHandler);
		if(regResult != ERROR_SUCCESS)
		{
			continue;
		}

		for(DWORD valueIndex = 0; ; ++valueIndex)
		{
			WCHAR valueNameBuf[16384];
			MemsetZero(valueNameBuf);
			DWORD valueNameBufLen = 16384;
			DWORD valueType = 0;
			BYTE valueData[16384];
			MemsetZero(valueData);
			DWORD valueDataLen = 16384;
			regResult = RegEnumValueW(hkHandler, valueIndex, valueNameBuf, &valueNameBufLen, NULL, &valueType, valueData, &valueDataLen);
			if(regResult != ERROR_SUCCESS)
			{
				break;
			}
			if(valueNameBufLen <= 0 || valueType != REG_SZ || valueDataLen <= 0)
			{
				continue;
			}

			std::wstring guid = std::wstring(valueNameBuf);

			mpt::ustring description = mpt::ToUnicode(std::wstring(reinterpret_cast<WCHAR*>(valueData)));
			description = mpt::String::Replace(description, MPT_USTRING("Byte Stream Handler"), MPT_USTRING("Files"));
			description = mpt::String::Replace(description, MPT_USTRING("ByteStreamHandler"), MPT_USTRING("Files"));

			guidMap[guid]
				.ShortName(MPT_USTRING("mf"))
				.Description(description)
				;

			if(handlerType[0] == L'.')
			{
				guidMap[guid].AddExtension(mpt::PathString::FromWide(handlerType.substr(1)));
			} else
			{
				guidMap[guid].AddMimeType(mpt::ToCharset(mpt::CharsetASCII, handlerType));
			}

		}

		RegCloseKey(hkHandler);
		hkHandler = NULL;

	}

	RegCloseKey(hkHandlers);
	hkHandlers = NULL;

	for(std::map<std::wstring, FileType>::const_iterator it = guidMap.begin(); it != guidMap.end(); ++it)
	{
		result.push_back(it->second);
	}

#endif // !NO_MEDIAFOUNDATION

	return result;
}
#endif // MPT_WITH_PATHSTRING
#endif // MODPLUG_TRACKER


bool CSoundFile::ReadMediaFoundationSample(SAMPLEINDEX sample, FileReader &file, bool mo3Decode)
//----------------------------------------------------------------------------------------------
{

#if defined(NO_MEDIAFOUNDATION)

	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(mo3Decode);
	return false;

#else

	ComponentHandle<ComponentMediaFoundation> mf;
	if(!IsComponentAvailable(mf))
	{
		return false;
	}

	file.Rewind();
	// When using MF to decode MP3 samples in MO3 files, we need the mp3 file extension
	// for some of them or otherwise MF refuses to recognize them.
	mpt::PathString tmpfileExtension = (mo3Decode ? MPT_PATHSTRING("mp3") : MPT_PATHSTRING("tmp"));
	OnDiskFileWrapper diskfile(file, tmpfileExtension);
	if(!diskfile.IsValid())
	{
		return false;
	}

	bool result = false;

	std::vector<char> rawData;
	FileTags tags;
	std::string sampleName;

	IMFSourceResolver *sourceResolver = NULL;
	MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
	IUnknown *unknownMediaSource = NULL;
	IMFMediaSource *mediaSource = NULL;
	IMFSourceReader *sourceReader = NULL;
	IMFMediaType *uncompressedAudioType = NULL;
	IMFMediaType *partialType = NULL;
	UINT32 numChannels = 0;
	UINT32 samplesPerSecond = 0;
	UINT32 bitsPerSample = 0;

	IMFSample *mfSample = NULL;
	DWORD mfSampleFlags = 0;
	IMFMediaBuffer *buffer = NULL;

	MPT_MF_CHECKED(MFCreateSourceResolver(&sourceResolver));
	MPT_MF_CHECKED(sourceResolver->CreateObjectFromURL(diskfile.GetFilename().AsNative().c_str(), MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE | MF_RESOLUTION_READ, NULL, &objectType, &unknownMediaSource));
	if(objectType != MF_OBJECT_MEDIASOURCE) goto fail;
	MPT_MF_CHECKED(unknownMediaSource->QueryInterface(&mediaSource));

	tags = ReadMFMetadata(mediaSource);

	MPT_MF_CHECKED(MFCreateSourceReaderFromMediaSource(mediaSource, NULL, &sourceReader));
	MPT_MF_CHECKED(MFCreateMediaType(&partialType));
	MPT_MF_CHECKED(partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	MPT_MF_CHECKED(partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	MPT_MF_CHECKED(sourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, partialType));
	MPT_MF_CHECKED(sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &uncompressedAudioType));
	MPT_MF_CHECKED(sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels));
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond));
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	if(numChannels <= 0 || numChannels > 2) goto fail;
	if(samplesPerSecond <= 0) goto fail;
	if(bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32) goto fail;

	while(true)
	{
		mfSampleFlags = 0;
		MPT_MF_CHECKED(sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &mfSampleFlags, NULL, &mfSample));
		if(mfSampleFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
		{
			break;
		}
		if(mfSampleFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			break;
		}
		MPT_MF_CHECKED(mfSample->ConvertToContiguousBuffer(&buffer));
		{
			BYTE *data = NULL;
			DWORD dataSize = 0;
			MPT_MF_CHECKED(buffer->Lock(&data, NULL, &dataSize));
			rawData.insert(rawData.end(), reinterpret_cast<char*>(data), reinterpret_cast<char*>(data + dataSize));
			MPT_MF_CHECKED(buffer->Unlock());
		}
		mptMFSafeRelease(&buffer);
		mptMFSafeRelease(&mfSample);
	}

	mptMFSafeRelease(&uncompressedAudioType);
	mptMFSafeRelease(&partialType);
	mptMFSafeRelease(&sourceReader);

	if(tags.artist.empty())
	{
		sampleName = mpt::ToCharset(mpt::CharsetLocale, tags.title);
	} else
	{
		sampleName = mpt::String::Print("%1 (by %2)", mpt::ToCharset(mpt::CharsetLocale, tags.title), mpt::ToCharset(mpt::CharsetLocale, tags.artist));
	}

	SmpLength length = rawData.size() / numChannels / (bitsPerSample/8);

	DestroySampleThreadsafe(sample);
	if(!mo3Decode)
	{
		mpt::String::Copy(m_szNames[sample], sampleName);
		Samples[sample].Initialize();
		Samples[sample].nC5Speed = samplesPerSecond;
	}
	Samples[sample].nLength = length;
	Samples[sample].uFlags.set(CHN_16BIT, bitsPerSample >= 16);
	Samples[sample].uFlags.set(CHN_STEREO, numChannels == 2);
	Samples[sample].AllocateSample();

	if(bitsPerSample == 24)
	{
		if(numChannels == 2)
		{
			CopyStereoInterleavedSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt24<0, littleEndian24> > >(Samples[sample], &rawData[0], rawData.size());
		} else
		{
			CopyMonoSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt24<0, littleEndian24> > >(Samples[sample], &rawData[0], rawData.size());
		}
	} else if(bitsPerSample == 32)
	{
		if(numChannels == 2)
		{
			CopyStereoInterleavedSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt32<0, littleEndian32> > >(Samples[sample], &rawData[0], rawData.size());
		} else
		{
			CopyMonoSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt32<0, littleEndian32> > >(Samples[sample], &rawData[0], rawData.size());
		}
	} else
	{
		// just copy
		std::copy(&rawData[0], &rawData[0] + rawData.size(), reinterpret_cast<char*>(Samples[sample].pSample));
	}

	result = true;

fail:

	mptMFSafeRelease(&buffer);
	mptMFSafeRelease(&mfSample);
	mptMFSafeRelease(&uncompressedAudioType);
	mptMFSafeRelease(&partialType);
	mptMFSafeRelease(&sourceReader);
	mptMFSafeRelease(&mediaSource);
	mptMFSafeRelease(&unknownMediaSource);
	mptMFSafeRelease(&sourceResolver);

	return result;

#endif

}


OPENMPT_NAMESPACE_END
